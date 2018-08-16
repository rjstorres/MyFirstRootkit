package communication;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.*;


public class TCPServer implements Runnable {

	ServerSocket socket;
	boolean quit = false;

	public TCPServer(int port) throws IOException {
		socket = new ServerSocket(port);
	}

	@Override
	public void run() {
		while (!quit) {
			try {
				Socket client = this.socket.accept();

				BufferedReader br = new BufferedReader(new InputStreamReader(client.getInputStream()));
				int b;
		        StringBuilder buf = new StringBuilder();
		        while ((b = br.read()) != -1) {
		            buf.append((char) b);
		        }

		        br.close();
		        new Thread(new MessageHandler(buf.toString(),CommunicationProtocol.IMALIVE,client.getInetAddress())).start();

			} catch (Exception e) {
				e.printStackTrace();
			}
		}

	}
	
	public void close() throws IOException {
		quit =true;
		socket.close();
	}

}