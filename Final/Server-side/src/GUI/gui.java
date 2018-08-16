package GUI;

import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import java.awt.Color;
import javax.swing.JTextField;
import javax.swing.JList;
import javax.swing.JLabel;
import javax.swing.JTextArea;
import javax.swing.JButton;
import java.awt.SystemColor;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.InetAddress;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;

import javax.swing.AbstractListModel;
import javax.swing.ButtonGroup;
import javax.swing.DefaultListModel;
import javax.swing.JScrollPane;
import java.awt.Font;
import javax.swing.event.ListSelectionListener;

import controller.Controller;
import controller.Database;
import utils.Pair;

import javax.swing.event.ListSelectionEvent;
import javax.swing.JCheckBox;

public class gui {

	private JFrame frame;
	private JTextField textField;
	private JButton btnNewButton;
	private JPanel panel_3;
	private JTextArea txtrAsda;
	private JScrollPane scrollPane_1;
	private JLabel lblResponses;
	private JPanel panel_2;
	private JTextArea txtrAsads;
	private JScrollPane scrollPane;
	private JLabel lblInputs;
	private JPanel panel_1;
	private JList list;
	private JPanel panel;
	private ButtonGroup group;
	private DefaultListModel<String> dlist;
	private static gui window = null;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					gui window = new gui();
					window.frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	public static gui getGui() {
		if (window == null) {
			window = new gui();
			window.frame.setVisible(true);
		}
		return window;
	}

	private gui() {
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	public void initialize() {
		frame = new JFrame();
		frame.setBounds(100, 100, 800, 600);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.getContentPane().setLayout(null);

		panel = new JPanel();
		panel.setBackground(SystemColor.control);
		panel.setBounds(0, 0, 137, 526);
		frame.getContentPane().add(panel);
		panel.setLayout(null);
		dlist = new DefaultListModel();
		list = new JList(dlist);

		list.setFont(new Font("Dialog", Font.BOLD, 11));
		list.setModel(new AbstractListModel() {
			String[] values = new String[] { "ALL" };

			public int getSize() {
				return values.length;
			}

			public Object getElementAt(int index) {
				return values[index];
			}
		});

		list.setSelectedIndex(0);
		list.setBounds(10, 11, 119, 503);
		panel.add(list);

		panel_1 = new JPanel();
		panel_1.setBounds(138, 0, 257, 526);
		frame.getContentPane().add(panel_1);
		panel_1.setLayout(null);
		// panel_1.add(TxtInputs);

		lblInputs = new JLabel(" Inputs");
		lblInputs.setFont(new Font("Times New Roman", Font.BOLD, 17));
		lblInputs.setBounds(0, 0, 259, 26);
		lblInputs.setForeground(Color.BLACK);
		lblInputs.setBackground(Color.WHITE);
		panel_1.add(lblInputs);

		scrollPane = new JScrollPane();
		lblInputs.setLabelFor(scrollPane);
		scrollPane.setBounds(0, 25, 279, 497);
		panel_1.add(scrollPane);

		txtrAsads = new JTextArea();
		txtrAsads.setEditable(false);
		txtrAsads.setFont(new Font("Dialog", Font.PLAIN, 12));
		scrollPane.setViewportView(txtrAsads);

		panel_2 = new JPanel();
		panel_2.setBounds(394, 0, 390, 526);
		frame.getContentPane().add(panel_2);
		panel_2.setLayout(null);

		lblResponses = new JLabel("Responses");
		lblResponses.setFont(new Font("Times New Roman", Font.BOLD, 17));
		lblResponses.setBounds(10, 0, 380, 25);
		panel_2.add(lblResponses);

		scrollPane_1 = new JScrollPane();
		lblResponses.setLabelFor(scrollPane_1);
		scrollPane_1.setBounds(10, 25, 380, 497);
		panel_2.add(scrollPane_1);

		txtrAsda = new JTextArea();
		txtrAsda.setEditable(false);
		txtrAsda.setFont(new Font("Dialog", Font.PLAIN, 12));
		scrollPane_1.setViewportView(txtrAsda);

		panel_3 = new JPanel();
		panel_3.setBounds(138, 531, 646, 31);
		frame.getContentPane().add(panel_3);
		panel_3.setLayout(null);

		btnNewButton = new JButton("Send");
		btnNewButton.setBounds(564, 0, 82, 28);
		panel_3.add(btnNewButton);

		textField = new JTextField();
		textField.setBounds(0, 0, 553, 29);
		panel_3.add(textField);
		textField.setColumns(10);

		JPanel panel_4 = new JPanel();
		panel_4.setBounds(0, 531, 137, 41);
		frame.getContentPane().add(panel_4);
		panel_4.setLayout(null);

		JRadioButton chckbxModule = new JRadioButton("module");
		chckbxModule.setBounds(53, 8, 76, 23);
		panel_4.add(chckbxModule);

		JRadioButton chckbxBash = new JRadioButton("bash");
		chckbxBash.setSelected(true);
		chckbxBash.setBounds(0, 8, 58, 23);
		panel_4.add(chckbxBash);

		group = new ButtonGroup();
		group.add(chckbxModule);
		group.add(chckbxBash);

		btnNewButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				String to_send = new String();
				if (chckbxModule.isSelected()) {
					to_send = "module_cmd:" + textField.getText();
				} else if (chckbxBash.isSelected()) {
					to_send = "bash_cmd:" + textField.getText();
				}
				txtrAsads.setText(txtrAsads.getText() + "\n\n" + "[" + (String) list.getSelectedValue() + " at "
						+ DateTimeFormatter.ofPattern("dd/MM/yyyy - hh:mm").format(ZonedDateTime.now()) + "]:\n"
						+ textField.getText());
				textField.setText("");
				Controller.send(to_send, list.getSelectedValue().toString());
			}
		});

		list.addListSelectionListener(new ListSelectionListener() {
			public void valueChanged(ListSelectionEvent arg0) {
				if (arg0.getValueIsAdjusting())
					return;

			}
		});

	}

	public void addListOption(String ip) {
		if (dlist.contains(ip)) {
			return;
		}

		dlist.addElement(ip);
	}

	public void updateResponses() {
		String to_set = new String();
		for (Pair<InetAddress, String> ip : Database.getResponses()) {
			if (((String) list.getSelectedValue()).equals("ALL")
					|| ((String) list.getSelectedValue()).equals(ip.getKey().getHostAddress()))
				to_set = ip.getValue() + "\n";
		}
		txtrAsda.setText(to_set);
	}

}