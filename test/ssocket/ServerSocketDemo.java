import javax.microedition.io.ServerSocketConnection;
import javax.microedition.io.SocketConnection;
import javax.microedition.io.Connector;
import java.io.DataInputStream;

class ServerSocketDemo {
	
	public static void main(String args[]) {
		
		try {
		
			ServerSocketConnection conn = (ServerSocketConnection)Connector.open("socket://:7879");
			System.out.println("Connector open");
			SocketConnection sc = (SocketConnection) conn.acceptAndOpen();
			System.out.println("Connected by client");
			DataInputStream is = sc.openDataInputStream();
			byte b = is.readByte();
			System.out.println(new Byte(b));
			System.out.println("Done!");
		}catch(Exception e) {

		}
	}
}
