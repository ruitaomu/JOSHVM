class HelloWorld {
	
	private int b (int a, int l) {
		while (l-- > 0) a = a*2;
		return a;
	}
	
	public static void main(String args[]) {
		
		HelloWorld h = new HelloWorld();
		int i = 7;
		System.out.println("HelloWorld from TEST begin");
		while (i-- > 0) {
			int c = h.b(i, 3);
			System.out.println("Hello, IOT world!" + i + "* 2^3 = " + c);

		}
		System.out.println("Waiting for 5 seconds...");
		try {
		Thread.sleep(5000);
		
		System.out.println("Hello again");
		System.out.println("Waiting for 5 seconds...");
		Thread.sleep(5000);
		System.out.println("Done!");
		}catch(Exception e) {
		}
	}
}
