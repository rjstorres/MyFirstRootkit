package controller;

import java.net.InetAddress;
import java.util.ArrayList;


import utils.Pair;


public class Database {
	private static ArrayList<Pair<InetAddress,Integer>> infectedComputers= new ArrayList<>();
	private static  ArrayList<Pair<InetAddress,String>> responses = new ArrayList<>();
	
	public static ArrayList<Pair<InetAddress, Integer>> getInfectedComputers() {
		return infectedComputers;
	}
	public static ArrayList<Pair<InetAddress,String>> getResponses() {
		return responses;
	}
	
	public static void addInfectedComputer(InetAddress addr,Integer port) {
		if(infectedComputers.contains(new Pair<>(addr,null)))
				infectedComputers.remove(new Pair<>(addr,null));
		infectedComputers.add(new Pair<>(addr,port));
		
	}
	
	
	public static void addResponse(InetAddress addr,String msg) {
		responses.add(new Pair<>(addr,msg));
	}
	
	
}
