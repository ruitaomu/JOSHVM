/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */
package org.joshvm.ams.jams;

import javax.microedition.io.*;
import java.io.*;
import javax.microedition.io.file.*;
import com.sun.cldc.io.j2me.file.Protocol;
import org.joshvm.security.internal.SecurityToken;
import com.sun.midp.log.*;

abstract class Installer {
    private static final int TYPE_APP = 0;
    private static final int TYPE_REG = 1;

    protected String installSource;
    protected String installDest;
    private SecurityToken securityToken;

    protected void init (SecurityToken securityToken, String installSource) {
        this.installSource = installSource;
        this.securityToken = securityToken;
    }

    protected Installer() {
    }

    public static Installer getInstaller (SecurityToken securityToken, String installSource) {
        Installer installer = null;
        Class clsInstaller;

        try {
            if (installSource.startsWith("file://")) {
                clsInstaller = Class.forName("org.joshvm.ams.jams.FileInstaller");
            } else if (installSource.startsWith("comm:")) {
                clsInstaller = Class.forName("org.joshvm.ams.jams.CommInstaller");
            } else if (installSource.startsWith("socket://")) {
                clsInstaller = Class.forName("org.joshvm.ams.jams.NetworkInstaller");
            } else {
                return null;
            }
            installer = (Installer)clsInstaller.newInstance();
            installer.init(securityToken, installSource);
        } catch (ClassNotFoundException cne) {
        } catch (InstantiationException ie) {
        } catch (IllegalAccessException iae) {
        }
        return installer;
    }

    public void install(String appName, String mainClass, int length, boolean autoStart) throws IOException, InstallVerifyErrorException {
        install(TYPE_APP, appName, mainClass, length, autoStart);
    }

    public void install(int type, String appName, String mainClass, int length, boolean autoStart)
            throws IOException, InstallVerifyErrorException {
        install(type, appName, mainClass, length, autoStart, null);
    }

    protected void install(int type, String appName, String mainClass, int length, boolean autoStart, AMSCallback requestNextBlockCallback)
            throws IOException, InstallVerifyErrorException {
        if (type == TYPE_REG) {
            installDest = "//"+Jams.getRegFilePath();
        } else {
            installDest = "//"+Jams.getAppdbRoot()+appName+".jar";
        }
        Protocol fconn = null;
        InputStream is;
        OutputStream file_os = null;
        try {
            Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Open file for write:" + installDest);
            fconn = new Protocol();
            fconn.openPrim(securityToken, installDest, Connector.READ_WRITE, false);
            if (fconn.exists()) {
                fconn.delete();
            }
            fconn.create();

            is = getSourceStream();
            int left;
            /*
            left = (is.read()&0xff)<<24;
            left |= (is.read()&0xff)<<16;
            left |= (is.read()&0xff)<<8;
            left |= (is.read()&0xff);
            */
            left = length;
            Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Expected download size:" + left);
            file_os = fconn.openOutputStream();
            final int MAX_LENGTH = 512;
            byte[] buf = new byte[MAX_LENGTH];
            boolean eof = false;
            while (!eof) {
                int total = 0;
                int size;
                if (left < MAX_LENGTH) {
                    size = left;
                } else {
                    size = MAX_LENGTH;
                }
                while ((total < size) && !eof) {
                    int count = is.read(buf, total, size - total);
                    if (count < 0) {
                        Logging.report(Logging.WARNING, LogChannels.LC_AMS, "Read EOF!!!");
                        eof = true;
                    } else if (count > 0) {
                        total += count;
                        left -= count;
                        if (left <= 0) {
                            eof = true;
                        }
                    } else {
                        try {
                            Thread.sleep(50);
                        } catch (InterruptedException e) {
                        }
                    }
                }
                file_os.write(buf, 0, total);
                if (requestNextBlockCallback != null) {
                    try {
                        requestNextBlockCallback.callback(left);
                    } catch (AMSException e) {
                        throw new IOException("IO Error when request file next block");
                    }
                }
                Logging.report(Logging.INFORMATION, LogChannels.LC_AMS, "Total left bytes to write:"+left);
            }

            if (type == TYPE_APP) {
                setAutoStart(appName, mainClass, autoStart);
            }
        } finally {
            if (file_os != null) {
                file_os.close();
            }
            if (fconn != null) {
                fconn.close();
            }
        }
    }

    public void installReg(int length) throws IOException, InstallVerifyErrorException {
        install(TYPE_REG, null, null, length, false);
    }

    protected abstract InputStream getSourceStream() throws IOException;

    private void setAutoStart(String appName, String mainClass, boolean autoStart) throws IOException {
        Protocol fconn = new Protocol();
        fconn.openPrim(securityToken, "//"+Jams.getAppdbRoot()+appName+".aut",
                        Connector.READ_WRITE, false);
        boolean exist = fconn.exists();
        if (autoStart) {
            if (!exist) {
                fconn.create();
            }
            OutputStream os = fconn.openOutputStream();
            os.write(mainClass.getBytes());
            os.close();
        } else {
            if (exist) {
                fconn.delete();
            }
        }
        fconn.close();
    }
}
