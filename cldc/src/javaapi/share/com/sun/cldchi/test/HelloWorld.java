package com.sun.cldchi.test;

import com.sun.midp.log.*;

class HelloWorld {
	private int b (int a, int l) {
		while (l-- > 0) a = a*2;
		return a;
	}
	
	public static void main(String args[]) {
		int j = 0;
		while(true) {
		HelloWorld h = new HelloWorld();
		int i = 7;
		Logging.report(Logging.ERROR, LogChannels.LC_CORE, "Test log");
		while (i-- > 0) {
			int c = h.b(i, 3);
			System.out.println("Hello, IOT world!"+j+++":" + i + "* 2^3 = " + c);

		}
		System.out.println("Free memory: "+Runtime.getRuntime().freeMemory());
		System.out.println("Time: " + System.currentTimeMillis());
		System.gc();
		System.out.println("Performing GC...");
		try {
			Thread.sleep(1000);
		}catch (InterruptedException ex) {
		}
		}
	}
}
