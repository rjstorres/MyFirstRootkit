package communication;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.nio.charset.Charset;

/**
 * The Class UDPServer.
 */
public class UDP extends Thread {

	/** The Constant MESSAGE_SIZE. */
	private static final int MESSAGE_SIZE = 65000;

	/** The socket. */
	private DatagramSocket socket;


	/**
	 * Instantiates a new UDP server.
	 *
	 * @param higherLayer the higher layer
	 * @param port        the port
	 */
	public UDP() {
		try {
			socket = new DatagramSocket();
		} catch (SocketException e) {
			e.printStackTrace();
			System.exit(-1);
		}
	}

	/*
	 * @see java.lang.Thread#run()
	 */
	public void run() {

		while (true) {
			/** The buf. */
			byte[] buf = new byte[MESSAGE_SIZE];
			DatagramPacket packet = new DatagramPacket(buf, buf.length);

			try {
				socket.receive(packet);
			} catch (IOException e) {
				e.printStackTrace();
			}
			
			String data = new String(packet.getData(), packet.getOffset(), packet.getLength(),
					Charset.forName("ISO_8859_1"));
			
			new Thread(new MessageHandler(data, CommunicationProtocol.RESPONSE, packet.getAddress())).start();
		}
	}

	/**
	 * Send UDP packet.
	 *
	 * @param msg the msg
	 * @return the string
	 */
	public String sendUDP(String msg, InetAddress address, int port) {
		byte[] buf = new byte[MESSAGE_SIZE];
		buf = msg.getBytes();
		DatagramPacket packet = new DatagramPacket(buf, buf.length, address, port);
		try {
			socket.send(packet);
			return msg;
		} catch (java.io.IOException e) {
			System.err.println(("Failed to send message to " + address + " on port " + port));
			return null;
		}
	}

	/**
	 * Close.
	 */
	public void close() {
		socket.close();
	}
}
