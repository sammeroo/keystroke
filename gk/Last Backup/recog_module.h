// GKRS - A simple Keystroke recognition module in GNOME
//
// AuThOrS:	Abhinav Choudhury
//		Sambit Chatterjee
//
// This header contains the neural network and/or classification 
// algorithm functions responsible for recognising a user's input.
////////////////////////////////////////////////////////

void _train(void)
{	// _train() will be called by on_Submit_Final_clicked() as the last step in Registration
	
	
}

gboolean _recognise(char *username)
{	// LUID (Login Username Input Data)
	// LPID (Login Password Input Data)
	// TEXT (keystroke Data of Entered Text)
	// username :: Actual username entered
	// The calling function, normally on_Authenticate_Button_clicked(), is responsible for checking 
	// if username/password validate/match and delegates recognition to this function.

}
