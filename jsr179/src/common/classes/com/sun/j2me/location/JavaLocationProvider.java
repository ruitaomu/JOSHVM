package com.sun.j2me.location;

import com.sun.j2me.location.*;
import javax.microedition.location.*;
import org.joshvm.util.ByteBuffer;
import org.joshvm.j2me.dio.*;
import org.joshvm.j2me.dio.spibus.*;
import java.util.Calendar;
import java.util.Date;


public class JavaLocationProvider extends LocationProviderImpl {
	Calendar calendar = Calendar.getInstance();
	private SPIDevice spi;
	//ByteBuffer buff = ByteBuffer.allocate(1024);
	byte[] str = new byte[1024];		
	byte[] receive_buff = new byte[1024];
	static LocationImpl lastLocation = null;
	static final LocationImpl defaultLocation = 
		new LocationImpl(new QualifiedCoordinates(0.0, 0.0, 0.0f, Float.NaN, Float.NaN), 0.0f, 0.0f, Location.MTE_SATELLITE, null, true);
	
    public int getState() { 
		if (spi == null) {
			return OUT_OF_SERVICE;
		} else {
			return AVAILABLE; 
		}
	}
	
    public int getDefaultInterval() { return 5;}

    public int getDefaultMaxAge() {return 10000;}

    public int getDefaultTimeout() {return 2;}

    public int getResponseTime() {return 1;}

    public int getStateInterval() {return 10;}

    public LocationImpl getLastLocation() {return lastLocation;}
	
	synchronized public static Location getLastKnownLocation() {
		if (lastLocation == null) {
			return defaultLocation;
		} else {
			return lastLocation;
		}
	}

	protected LocationImpl updateLocation(long timeout) throws LocationException {
		String nmea = getNMEAString();
		parseNMEAString(nmea);
		if (nmea != null) {
			lastLocation.extraInfoNMEA = nmea;
			return lastLocation;
		}
		throw new LocationException("Can't get location information from device!");
	}

	public JavaLocationProvider(String name) throws IllegalAccessException {
		try {
			SPIDeviceConfig config = new SPIDeviceConfig.Builder()
									.setControllerNumber(0)
									.setAddress(0)
									.setClockFrequency(25000)
									.setClockMode(0)
									.setWordLength(8)
									.setBitOrdering(Device.BIG_ENDIAN)
									.setCSActiveLevel(SPIDeviceConfig.CS_ACTIVE_LOW)
									.build();
			spi = (SPIDevice)DeviceManager.open(config, DeviceManager.EXCLUSIVE);
			lastLocation = new LocationImpl(new QualifiedCoordinates(0.0, 0.0, 0.0f, Float.NaN, Float.NaN), 0.0f, 0.0f, Location.MTE_SATELLITE, null, true);
		} catch (Exception e) {
			e.printStackTrace();
			throw new IllegalAccessException("");
		}
		
		lastLocation = defaultLocation;
	}

	public static String getListOfLocationProviders() {
		return "ublox-6M-SPI";
	}

	private String getNMEAString() {
		try {
		if (spi != null) {
			ByteBuffer buff = ByteBuffer.wrap(receive_buff);
			spi.read(buff);
			buff.flip();
			int pos = 0;
			while (buff.hasRemaining()) {
				byte b = buff.get();
				if (b != -1) {
					str[pos++] = b;
				} else {
					str[pos] = 0;
				}
			}
			String out = new String(str);
			return out;
		} else {
			return null;
		}
		}catch (Exception e) {
			e.printStackTrace();
		}

		
		return null;
	}	

	private void parseNMEAString(String s) {
		int pos = 0;
		int occur;
		while ((occur = s.indexOf("\r\n", pos)) != -1) {
			parseNMEALine(s.substring(pos, occur));
			pos = occur + 2;
		}
	}

	private void parseNMEALine(String s) {
		int i = 0;
		int pos = 0;
		int nextPos = 0;
		String[] data = new String[25];
		
		try {
			while (true) {
				nextPos = s.indexOf(",", pos);
				if (nextPos < 0) {
					nextPos = s.indexOf("*", pos);
					if (nextPos < 0) {
						break;
					}
				}
				data[i++] = s.substring(pos, nextPos);
				pos = nextPos + 1;
				if (i >= 25) {
					break;
				}
			}
			if (data[0].startsWith("$GPGGA")) {
				// satelites
				if (data[7].length() > 0) {
					//sat = Integer.parseInt(data[7]);
				}
				// coordinates
				if ((data[9].length() > 0) && (data[2].length() > 0) && (data[4].length() > 0)) {
					int pos_dot;
					
					// altitude
					float alt;
					try {
						alt = Float.parseFloat(data[9]);
					} catch (NumberFormatException e) {
						alt = 0.0f;
					}
					
					// latitude
					double lat;
					pos_dot = data[2].indexOf(".");
					if ((pos_dot == -1) || (pos_dot < 2)) {
						lat = 0.0;
					} else {
						try {
							int lat_int = Integer.parseInt(data[2].substring(0, pos_dot-2));
							double lat_frc = Double.parseDouble(data[2].substring(pos_dot-2));
							lat_frc = lat_frc / 60.0;
							lat = lat_int + lat_frc;
						} catch (NumberFormatException e) {
							lat = 0.0;
						}
					}
					if (data[3].equals("S")) {
						lat = -lat;
					}
					
					// longitude
					double lon;
					pos_dot = data[4].indexOf(".");
					if ((pos_dot == -1) || (pos_dot < 2)) {
						lon = 0.0;
					} else {
						try {
							int lon_int = Integer.parseInt(data[4].substring(0, pos_dot-2));
							double lon_frc = Double.parseDouble(data[4].substring(pos_dot-2));
							lon_frc = lon_frc / 60.0;
							lon = lon_int + lon_frc;
						} catch (NumberFormatException e) {
							lon = 0.0;
						}
					}
					if (data[5].equals("W")) {
						lon = -lon;
					}
					// Debug.debug( s );
					lastLocation.setQualifiedCoordinates(new QualifiedCoordinates(lat, lon, alt, Float.NaN, Float.NaN));
				}

				//parseTimeGGA = System.currentTimeMillis() - parseTimeStart;
			} else if (data[0].startsWith("$GPRMC")) {
				try {
					// time
					int raw = Integer.parseInt(data[1].substring(0, 6));
					calendar.setTime(new Date((long) (raw % 100 + ((raw / 100) % 100) * 60 + (raw / 10000) * 3600) * 1000));
					// date
					raw = Integer.parseInt(data[9]);
					calendar.set(Calendar.YEAR, raw % 100 + 2000);
					calendar.set(Calendar.MONTH, (raw / 100) % 100 - 1);
					calendar.set(Calendar.DAY_OF_MONTH, raw / 10000 + calendar.get(Calendar.DAY_OF_MONTH) - 1);
				} catch (Exception e) {
				}
				if ((data[7].length() > 0) && (data[8].length() > 0)) {
					// speed
					float speed;
					try {
						speed = Float.parseFloat(data[7]) * 1852 / 3600;
					} catch (NumberFormatException e) {
						speed = 0.0f;
					}
					lastLocation.setSpeed(speed);
					// course
					float course;
					try {
						course = Float.parseFloat(data[8]);
					} catch (NumberFormatException e) {
						course = 0.0f;
					}
					lastLocation.setCourse(course);
				}
				//parseTimeRMC = System.currentTimeMillis() - parseTimeStart;
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	

}

