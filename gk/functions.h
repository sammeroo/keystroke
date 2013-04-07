// GKRS - A simple Keystroke recognition module in GNOME
//
// AuThOrS:	Abhinav Choudhury
//
// This module is mainly responsible for the GUI of the program
// This module also calculates secondary keystroke data and
// is responsible for maintaining data/file consistency.
////////////////////////////////////////////////////////

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>
#include "recog_module.h"
#include "auxiliary.h"
#include "KNN.h"

#define	VALID 0
#define INVALID 1

#define KNNVAL	15

int LEVEL;		// LEVEL is the security level read from the configuration file "settings.conf"
			// by the function get_conf_settings(). If "settings.conf" does not exist, default = 4
int LOCK;		// Used to synch compute_func2

GtkBuilder 	*builder_main, 
		*builder_login, 
		*builder_register,
		*builder_analysis,
		*builder_settings;

GtkWidget 	*window_main, 		// For main window widget
		*window_login, 		// For login window widget
		*window_register,	// For register window widget
		*window_analysis,
		*window_settings;

GError     *error = NULL;

struct registered_users {
	int N;
	char **name;
};

struct top_users  {
	int n;
	char **user;
};

void compute_func(FILE *F);	// Calculates necessary data from kdata_usr and kdata_pass and puts them in FINAL_TEMPLATE
int get_usr_log_files(void);			// This function outputs all user log files, except kdata files, writing their names into 'list' file
						// returns 1 if successful, 0 if no log files found
struct top_users return_top_users(FILE *knnfile);

void compute_func2(FILE *F);
int create_merged_log_file(void);
int create_list_of_deviations(void);
double scale(double d);			// Scale all generated values to a double fraction value within [-10 to 10]
void clean_tmp_folder(void);
double* feed_into_network(FILE *F);
void create_deviations_final_file(char *name);
int* get_KNNVAL(void);			// Calculates the value of the KNNVAL parameter which is used as input to the KNN module
struct registered_users* get_registered_usernames(void);
void create_new_rule_devfile(void);

struct input {
	char type[10];
	gchar key;
	guint time;
	int shift;
	int caps;
} *in_data;

struct template_input {		// SUBJECT TO CHANGE
	double MHT_U;
	double MLT_U;
	double MHT_P;
	double MLT_P;
	double ATS;
	double SD;
	double GF;
} *in;

struct devfile_input {
	double dev1;
	double dev2;
	double dev3;
	double dev4;
	double dev5;
	double dev6;
	double dev7;
	char name[120];
};

// List of all output files created
FILE *f;
FILE *kdata_usr;	// Temp file for storing username keystroke data from Username_Entry1
FILE *kdata_pass;	// Temp file for storing password keystroke data from Password_Entry1
FILE *FINAL_TEMPLATE;		// Final data after calculations on_SUbmit_Button_Register_pressed
FILE *TEMPLOGIN;		// Temporary file used during login

gboolean Registration_Complete;		// Marks the end of registration, initialised to FALSE during on_Register_Button_clicked
gboolean Login_Complete;		// Marks the end of login, initialised to FALSE during on_Login_Button_clicked

gint username_pos = 0;
gint password_pos = 0;

// last_usr and last_pass are used in the registration phase to check whether last usrname/password match currently entered username/password
gchar last_usr[100];
gchar last_pass[100];
char login_attempt_usr[120];		// The username entered during login attempt

G_MODULE_EXPORT gboolean on_text_view_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer userdata)
{
//	time(&tm);
	GdkEventKey kp;
	kp = event->key;
	fprintf(f, "PRESS  %d %u %d %d %d\n", kp.keyval, kp.time, kp.state&1, kp.state&2, kp.state&4);
}

G_MODULE_EXPORT gboolean on_text_view_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer userdata)
{
//	time(&tm);
	GdkEventKey kr;
	kr = event->key;
	fprintf(f, "RELEASE %d %u %d %d %d\n", kr.keyval, kr.time, kr.state&1, kr.state&2, kr.state&4);
}

G_MODULE_EXPORT void on_Main_Window_destroy(void)	// When main window is closed
{
	gtk_main_quit();
}

G_MODULE_EXPORT void on_Login_Button_clicked(GtkButton *button, gpointer userdata)	// When login button is clicked
{
	Login_Complete = FALSE;
	builder_login = gtk_builder_new();
	
	if( ! gtk_builder_add_from_file( builder_login, "login.glade", &error ) )
	{
	        g_warning( "%s", error->message );
        	g_free( error );
        	return;
	}
	window_login = GTK_WIDGET(gtk_builder_get_object(builder_login, "Login_Window"));
	gtk_builder_connect_signals(builder_login, NULL);
	gtk_widget_show(window_login);
}

G_MODULE_EXPORT void on_Register_Button_clicked(GtkButton *button, gpointer userdata)	// When register button is clicked
{
	Registration_Complete = FALSE;
	builder_register = gtk_builder_new();

	if( ! gtk_builder_add_from_file( builder_register, "register.glade", &error ) )
	{
	        g_warning( "%s", error->message );
        	g_free( error );
        	return;
	}
	window_register = GTK_WIDGET(gtk_builder_get_object(builder_register, "Register_Window"));
	gtk_builder_connect_signals(builder_register, NULL);
	gtk_widget_show(window_register);
}

G_MODULE_EXPORT void on_Analysis_Button_clicked(void)
{
	builder_analysis = gtk_builder_new();
	if( ! gtk_builder_add_from_file( builder_analysis, "analysis.glade", &error ) )
	{
	        g_warning( "%s", error->message );
        	g_free( error );
        	return;
	}
	window_analysis = GTK_WIDGET(gtk_builder_get_object(builder_analysis, "Analysis_Window"));
	gtk_builder_connect_signals(builder_analysis, NULL);
	gtk_widget_show(window_analysis);
}

G_MODULE_EXPORT void on_Settings_Button_clicked(void)
{
	builder_settings = gtk_builder_new();

	if( ! gtk_builder_add_from_file( builder_settings, "settings.glade", &error ) )
	{
	        g_warning( "%s", error->message );
        	g_free( error );
        	return;
	}
	window_settings = GTK_WIDGET(gtk_builder_get_object(builder_settings, "Settings_Window"));
	gtk_builder_connect_signals(builder_settings, NULL);
	gtk_widget_show(window_settings);	
}

G_MODULE_EXPORT void on_Login_Window_destroy(void)		// When login window is closed
{
	if(Login_Complete == FALSE)
	{	// Remove all temporary files : kdata_usr, kdata_pass
		remove("log/kdata_usr.log");
		remove("log/kdata_pass.log");
	}
	gtk_widget_destroy(window_login);
}

G_MODULE_EXPORT void on_Settings_Window_destroy(void)
{

}

G_MODULE_EXPORT void on_Exit_Button_clicked(void)
{
	gtk_main_quit();
}

void on_Enter_Button_clicked()
{	// This signal handling function does the following:
	// 	1. Check if Username and Password size are >= 10, if not resets both fields
	//	2. Check if Username entered is already registered by searching for a file named "Username.log" in the current directory
	//	3. If Username exists, Password entered is matched for that Username.
	// 	4. Next, keystroke recognition proceeds.
	// 	5. If all 3 steps are verified, the user is authenticated	
	//   ***6. Also, if authenticated, record currently captured keystroke data in FINAL_TEMPLATE as TRUE
	//    **7. or if not authenticated, record currently captured keystroke data in FINAL_TEMPLATE as FALSE 
	
	GtkLabel *usrl = (GtkLabel*)GTK_WIDGET(gtk_builder_get_object(builder_login, "Username_Label"));
	GtkLabel *passl = (GtkLabel*)GTK_WIDGET(gtk_builder_get_object(builder_login, "Password_Label"));
	GtkEntry *usr = (GtkEntry *)GTK_WIDGET(gtk_builder_get_object(builder_login, "Username_Entry"));
	GtkEntry *pass = (GtkEntry *)GTK_WIDGET(gtk_builder_get_object(builder_login, "Password_Entry"));
	GtkLabel *label = (GtkLabel *)GTK_WIDGET(gtk_builder_get_object(builder_login, "Login_Status"));
	GtkLabel *label2 = (GtkLabel *)GTK_WIDGET(gtk_builder_get_object(builder_login, "Large_Entry_Label"));
	GtkTextView *text = (GtkTextView *)GTK_WIDGET(gtk_builder_get_object(builder_login, "Login_Large_Entry"));
	GtkButton *button = (GtkButton*)GTK_WIDGET(gtk_builder_get_object(builder_login, "Authenticate_Button"));

	if(gtk_entry_get_text_length(usr) <10)
	{	gtk_label_set_text(label, "Username should be atleast 10 characters.");
		gtk_editable_delete_text((GtkEditable*)usr, 0, -1);
		gtk_editable_delete_text((GtkEditable*)pass, 0, -1);
		remove("log/kdata_usr.log");
		remove("log/kdata_pass.log");
	}
	else if(gtk_entry_get_text_length(pass) < 10)
	{	gtk_label_set_text(label, "Password should be atleast 10 characters.");
			gtk_editable_delete_text((GtkEditable*)usr, 0, -1);
			gtk_editable_delete_text((GtkEditable*)pass, 0, -1);
			remove("log/kdata_usr.log");
			remove("log/kdata_pass.log");
	}
	else if(gtk_entry_get_text_length(usr) < 10 && gtk_entry_get_text_length(pass) < 10)
	{	gtk_label_set_text(label, "Username/Password should be atleast 10 characters.");
			gtk_editable_delete_text((GtkEditable*)usr, 0, -1);
			gtk_editable_delete_text((GtkEditable*)pass, 0, -1);
			remove("log/kdata_usr.log");
			remove("log/kdata_pass.log");
	}
	else	// Condition 1 checked and passed
	{
		FILE *tmp;		
		char buffer[110];
		strcpy(login_attempt_usr, gtk_entry_get_text(usr));
		sprintf(buffer, "log/%s.log", login_attempt_usr);
		if((tmp = fopen(buffer, "r")) == NULL)
		{
			gtk_label_set_text(label, "Username entered is not registered.\nRegister first or enter a different username.");
			gtk_editable_delete_text((GtkEditable*)usr, 0, -1);
			gtk_editable_delete_text((GtkEditable*)pass, 0, -1);
			remove("log/kdata_usr.log");
			remove("log/kdata_pass.log");
		}
		// Else check for password match, and proceed accordingly
		else
		{
			// Scan user's registered password from tmp into a buffer
			char p[100], sc[100];
			strcpy(p, gtk_entry_get_text(pass));
			fscanf(tmp, "%s %s", sc, sc);

			if( strcmp(sc, p) == 0)
			{
				gtk_widget_hide((GtkWidget*)usrl);
				gtk_widget_hide((GtkWidget*)passl);
				gtk_widget_hide((GtkWidget*)usr);
				gtk_widget_hide((GtkWidget*)pass);
				gtk_widget_show((GtkWidget*)label2);
				gtk_widget_show((GtkWidget*)text);
				gtk_widget_show((GtkWidget*)button);
				gtk_label_set_text(label, "Enter the text above in your normal typing pace.");
				TEMPLOGIN = fopen("tmp/templogin", "w");
				kdata_usr = fopen("log/kdata_usr.log", "r");
				kdata_pass = fopen("log/kdata_pass.log", "r");
				compute_func(TEMPLOGIN);
				while(LOCK);
				fclose(TEMPLOGIN);
			}
			else; // Password does not match
		}
		fclose(tmp);
	}
}

G_MODULE_EXPORT gboolean on_Username_Entry_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer userdata)
{
	GdkEventKey eventkey = event->key;
	GtkEntry *username = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Username_Entry"));
	GtkEntry *password = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Password_Entry"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval) || eventkey.keyval == GDK_underscore)
	{
		GtkEntryBuffer *usrbuf = gtk_entry_get_buffer(username);
		gchar ch[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(ch, "_");
		else	strcpy(ch, gdk_keyval_name(eventkey.keyval));
/*
printf("ch = %s ", ch);
//		gint pos = gtk_editable_get_position((GtkEditable*)username);
		gint pos = gtk_entry_get_text_length(username);
printf("pos before = %d ", pos);
//		gtk_editable_set_position((GtkEditable*)username, -1);
//		gtk_editable_insert_text((GtkEditable*)username, ch, -1, &pos);
//		gtk_entry_buffer_insert_text(usrbuf, pos, ch, -1);
//		gtk_entry_set_buffer(username, usrbuf);
printf("pos after = %d\n", pos);
//		gtk_entry_append_text(username, ch);
*/
		// CODE to output keystroke data into file kdata_usr
		kdata_usr = fopen("log/kdata_usr.log", "a");
		fprintf(kdata_usr, "press %s %u %d %d\n", ch, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_usr);
	}
	else if(eventkey.keyval == GDK_BackSpace || eventkey.keyval == GDK_Delete)	// If backspace or delete os pressed, reset fields and empty datalog files
	{
		gtk_entry_set_text(username, "");
		remove("log/kdata_usr.log");
		kdata_usr = fopen("log/kdata_usr.log", "a");
	}
	else if(eventkey.keyval == GDK_Return || eventkey.keyval == GDK_Tab)
		gtk_widget_grab_focus((GtkWidget*) password);		
	else ;	// Any other character entered entered in username field, do nothing
	{
//		gtk_label_set_text(status_box, "Invalid character for username. Please enter again.");
		// Enter code to reset data previously captured before invalid character and reset username field to empty
	}

}

G_MODULE_EXPORT gboolean on_Username_Entry_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer userdata)
{
	GdkEventKey eventkey = event->key;
	GtkEntry *username = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Username_Entry"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval) || eventkey.keyval == GDK_underscore)
	{
		gchar chr[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(chr, "_");
		else	strcpy(chr, gdk_keyval_name(eventkey.keyval));

		kdata_usr = fopen("log/kdata_usr.log", "a");
		fprintf(kdata_usr, "release %s %u %d %d\n", chr, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_usr);
	}
	else ;	// Any other character entered entered in username field, do nothing
}

G_MODULE_EXPORT gboolean on_Password_Entry_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer userdata)
{
	GdkEventKey eventkey = event->key;
	GtkEntry *password = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Password_Entry"));
	GtkEntry *but = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Enter_Button"));
        GtkEntry *username = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Username_Entry"));
	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval))
	{
		gchar ch[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(ch, "_");
		else	strcpy(ch, gdk_keyval_name(eventkey.keyval));
/*
		gchar t[2];
		strcpy(t, "*");
//		gint pos = gtk_editable_get_position((GtkEditable*)username);
		gint pos = gtk_entry_get_text_length(password);
		gtk_editable_insert_text((GtkEditable*)password, ch, strlen(ch), &pos);
*/
		// CODE to output keystroke data into file
		kdata_pass = fopen("log/kdata_pass.log", "a");
		fprintf(kdata_pass, "press %s %u %d %d\n", ch, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_pass);
	}
	else if(eventkey.keyval == GDK_BackSpace || eventkey.keyval == GDK_Delete)	// If backspace or delete os pressed, reset fields and empty datalog files
	{
		gtk_entry_set_text(password, "");
		remove("kdata_pass.log");
		kdata_usr = fopen("log/kdata_pass.log", "a");
	}
	else if(eventkey.keyval == GDK_Tab)
		gtk_widget_grab_focus((GtkWidget*) but);
	else if(eventkey.keyval == GDK_Return)
		{on_Enter_Button_clicked();
		gtk_widget_grab_focus((GtkWidget*) username);
       }
	else;	// Any other character entered entered in username field, do nothing
}

G_MODULE_EXPORT gboolean on_Password_Entry_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer userdata)
{
	GdkEventKey eventkey = event->key;
	GtkEntry *password = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Password_Entry"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval))
	{
		gchar ch[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(ch, "_");
		else	strcpy(ch, gdk_keyval_name(eventkey.keyval));

		kdata_pass = fopen("log/kdata_pass.log", "a");
		fprintf(kdata_pass, "release %s %u %d %d\n", ch, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_pass);
	}
	else ;	// Any other character entered entered in username field, do nothing
}

G_MODULE_EXPORT void on_Submit_Button_Register_clicked(void)
{
	gdouble current_fraction;		// Current fraction of Progress_Bar filled
	GtkProgressBar *Progress_Bar;
	GtkEntry *username, *password;
	GtkLabel *status_box;
	gchar current_usr[100], current_pass[100];	// Holds currently entered username and password

	Progress_Bar = (GtkProgressBar *) GTK_WIDGET(gtk_builder_get_object(builder_register, "Progress_Bar"));	// Get handle to Progress_Bar
	username = (GtkEntry *) GTK_WIDGET(gtk_builder_get_object(builder_register, "Username_Entry1"));	// Get handle to Username widget
	password = (GtkEntry *) GTK_WIDGET(gtk_builder_get_object(builder_register, "Password_Entry1"));	// Get handle to Password widget
	status_box = (GtkLabel *) GTK_WIDGET(gtk_builder_get_object(builder_register, "Status_Display"));	// Get handle to status_display
	GtkButton *b = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder_register, "Submit_Button_Register"));

//	char temp[100];
//	sprintf(temp, "last_usr = %s, last_pass = %s", last_usr, last_pass);
//	gtk_label_set_text(status_box, temp);

	current_fraction = gtk_progress_bar_get_fraction(Progress_Bar);			// Get current fraction filled
	strcpy(current_usr, gtk_entry_get_text(username));
	strcpy(current_pass, gtk_entry_get_text(password));

	int i = gtk_entry_get_text_length(username);
	int j = gtk_entry_get_text_length(password);

	if( i >= 10 && j >= 10 )		// Include checks for fields empty/not empty
	{
		gchar buf[50];
		if(current_fraction == 0.0) {		// For the first entry no need to match with last_usr and last_pass
			gtk_entry_set_text(username, "");	// Empty the username field
			gtk_entry_set_text(password, "");	// Empty the password field
//			gtk_progress_bar_set_fraction(Progress_Bar, current_fraction+0.1);	// Increase Progress_Bar by fraction 0.1
			gtk_progress_bar_set_fraction(Progress_Bar, current_fraction + ((double)1/(double)(2*LEVEL)));
//			sprintf(buf, "Training Progress: %d/10", (int)((current_fraction+0.1)*10));	// Set progress bar text
			sprintf(buf, "Training Progress: %1.2lf/%d", (double)(current_fraction + ((double)1/(double)(2*LEVEL)))*(double)(2*LEVEL), 2*LEVEL);
			gtk_progress_bar_set_text(Progress_Bar, buf);
			strcpy(last_usr, current_usr);		// Set last_usr = current_usr
			strcpy(last_pass, current_pass);	// Set last_pass = current_pass

			// Enter CODE to convert raw data from kdata_usr and kdata_pass files to FINAL_TEMPLATE file
			// ** Also enter Username and Password(Username optional, since filename already has it) into FINAL_TEMPLATE for record-keeping

//			kdata_usr = fopen("log/kdata_usr.log", "a");
//			kdata_pass = fopen("log/kdata_pass.log", "a");
//			fprintf(kdata_usr, "END\n");			// Mark end of both files
//			fprintf(kdata_pass, "END\n");			

			kdata_usr = fopen("log/kdata_usr.log", "r");
			kdata_pass = fopen("log/kdata_pass.log", "r");
			gchar usrfile[100];
			sprintf(usrfile, "log/%s.log", current_usr);	// usrfile = "<username>.log"
			FINAL_TEMPLATE = fopen(usrfile, "a");		// Open final template file with name "<username>.log"
			fprintf(FINAL_TEMPLATE, "password %s\n", current_pass);		// First line of template file : "password <password>"
			compute_func(FINAL_TEMPLATE);
			fclose(FINAL_TEMPLATE);
			remove("log/kdata_usr.log");
			remove("log/kdata_pass.log");
		}
		else {
			if(strcmp(current_usr, last_usr)==0 && strcmp(current_pass, last_pass)==0) {	// If current entries match last etries
				gtk_entry_set_text(username, "");
				gtk_entry_set_text(password, "");
//				gtk_progress_bar_set_fraction(Progress_Bar, current_fraction+0.1);	// Increase Progress_Bar by fraction 0.1
			gtk_progress_bar_set_fraction(Progress_Bar, current_fraction + ((double)1/(double)(2*LEVEL)));
//				sprintf(buf, "Training Progress: %d/10", (int)((current_fraction+0.1)*10));	// Set progress bar text
			sprintf(buf, "Training Progress: %1.2lf/%d", (double)(current_fraction + ((double)1/(double)(2*LEVEL)))*(double)(2*LEVEL), 2*LEVEL);
				gtk_progress_bar_set_text(Progress_Bar, buf);
				strcpy(last_usr, current_usr);		// Set last_usr = current_usr
				strcpy(last_pass, current_pass);	// Set last_pass = current_pass

				// Enter CODE to convert raw data from kdata_usr and kdata_pass files to FINAL_TEMPLATE file

//				kdata_usr = fopen("log/kdata_usr.log", "a");
//				kdata_pass = fopen("log/kdata_pass.log", "a");
//				fprintf(kdata_usr, "END\n");			// Mark end of both files
//				fprintf(kdata_pass, "END\n");			

				kdata_usr = fopen("log/kdata_usr.log", "r");
				kdata_pass = fopen("log/kdata_pass.log", "r");
				gchar usrfile[100];
				sprintf(usrfile, "log/%s.log", current_usr);	// usrfile = "<username>.log"
				FINAL_TEMPLATE = fopen(usrfile, "a");		// Open final template file with name "<username>.log"
				compute_func(FINAL_TEMPLATE);
				fclose(FINAL_TEMPLATE);
				remove("log/kdata_usr.log");
				remove("log/kdata_pass.log");
			}
			else {
				// Current entries do not match last entries
				gtk_label_set_text(status_box, "Username and/or password does not match!\nPlease enter again.");
				gtk_entry_set_text(username, "");
				gtk_entry_set_text(password, "");
				remove("log/kdata_usr.log");
				remove("log/kdata_pass.log");				
			}
		}
	}
	else if(i<10) {
		gtk_label_set_text(status_box, "NOTE: Username should be atleast 10 characters.");
		gtk_entry_set_text(username, "");
		gtk_entry_set_text(password, "");
		remove("log/kdata_usr.log");
		remove("log/kdata_pass.log");				
	}
	else if(j<10) {
		gtk_label_set_text(status_box, "NOTE: Password should be atleast 10 characters.");		
		gtk_entry_set_text(username, "");
		gtk_entry_set_text(password, "");
		remove("log/kdata_usr.log");
		remove("log/kdata_pass.log");				
	}

	if(gtk_progress_bar_get_fraction(Progress_Bar) == 1.0)		// If progress bar filled
	{
		gtk_editable_set_editable((GtkEditable*)username, FALSE);
		gtk_editable_set_editable((GtkEditable*)password, FALSE);
		gtk_widget_hide((GtkWidget*)b);
		GtkLayout *layout2 = (GtkLayout *) GTK_WIDGET(gtk_builder_get_object(builder_register, "layout2"));
		GtkLabel *label2 = (GtkLabel *) GTK_WIDGET(gtk_builder_get_object(builder_register, "label2"));		
		gtk_label_set_text(status_box, "Thank You! Please continue Training Phase II.\nNote: Try to keep a normal/steady typing pace.");
		gtk_widget_show_all((GtkWidget*)layout2);
//		gtk_label_set_text(status_box, "");
	}
}

void on_Submit_Button_R_clicked(void)
{
	GtkLabel *status_box = (GtkLabel*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Status_Display"));
	GtkTextView *Large_Text = (GtkTextView*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Large_Entry1"));
	GtkTextIter start, end;
	GtkTextBuffer *buf = gtk_text_view_get_buffer(Large_Text);
	gtk_text_buffer_get_iter_at_offset(buf, &start, 0);
	gtk_text_buffer_get_iter_at_offset(buf, &end, -1);
	char *text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
	char text1[] = "The quick brown fox jumps over the lazy dog";
	char text2[] = "the quick brown fox jumps over the lazy dog";
/*
	if(strcmp(text, text1)==0 || strcmp(text, text2)==0)	// If text entered matches given text,
	{
*/		GtkLabel *label4 = (GtkLabel*) GTK_WIDGET(gtk_builder_get_object(builder_register, "label4"));
		GtkButton *button1 = (GtkButton*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Submit_Button_R"));
		GtkButton *button2 = (GtkButton*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Submit_Final"));

		GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
		gtk_text_buffer_set_text(buffer, "", 0);
		gtk_text_view_set_buffer(Large_Text, buffer);
	
		gchar *markup_text;
		gchar str[100];
		strcpy(str, "The wizard quickly jinxed the gnomes before they vaporized.");
		markup_text = g_markup_printf_escaped ("<span style=\"italic\" color=\"#ff0000\">%s</span>", str);
		gtk_label_set_markup(GTK_LABEL(label4), markup_text);
		
		gtk_widget_hide((GtkWidget*)button1);
		gtk_widget_destroy((GtkWidget*)button1);
		gtk_widget_show((GtkWidget*)button2);
	
		// Enter code for computation on LargeEntry1 data collected into kdata_usr.log
		char dummy[120];
		sprintf(dummy, "log/%s.log", last_usr);

//		kdata_usr = fopen("log/kdata_usr.log", "r");		
//		FINAL_TEMPLATE = fopen(dummy, "a");
//		if(FINAL_TEMPLATE == NULL)	{	printf("Error. User log file was not found!");	return;	}

//		compute_func2(FINAL_TEMPLATE);
/*	}
	else		// If text entered does not match given text
	{
		gtk_text_buffer_set_text(buf, "", sizeof(""));
		gtk_label_set_text(status_box, "Data not entered correctly. Please re-enter.");
	}	
*/
	remove("log/kdata_usr.log");	
}

void on_Register_Window_destroy(void)
{
	if( Registration_Complete == FALSE)
	{
		// If registration is incomplete and window close is attempted, rollback all changes and remove partially gathered data
		// and alert user. Among rolling back changes, delete the FINAL_TEMPLATE file i.e. the file named "<last_usr>.log",
		// which is the partially complete template file.

		if(remove("log/kdata_usr.log") == -1);
		if(remove("log/kdata_pass.log") == -1);
		if(strlen(last_usr) >= 10)	// If last_usr is not empty
		{
			gchar buf[110];
			sprintf(buf, "log/%s.log", last_usr);
			remove(buf);
		}
	}

	gtk_widget_destroy(window_register);
}

void on_Submit_Final_clicked(void)
{printf("submit entered...\n");
	GtkLabel *status_box = (GtkLabel*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Status_Display"));
	GtkTextView *Large_Text = (GtkTextView*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Large_Entry1"));
	GtkTextIter start, end;
	GtkTextBuffer *buf = gtk_text_view_get_buffer(Large_Text);
	gtk_text_buffer_get_iter_at_offset(buf, &start, 0);
	gtk_text_buffer_get_iter_at_offset(buf, &end, -1);
	char *text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
	char text1[] = "The wizard quickly jinxed the gnomes before they vaporized";
	char text2[] = "the wizard quickly jinxed the gnomes before they vaporized";
	
//	if(strcmp(text, text1) == 0 || strcmp(text, text2) == 0) {		// Data is correctly in Large_Text1
		Registration_Complete = TRUE;
		// First store data gathered from Large_Text1 in file and then _train()
		char dummy[120];
		sprintf(dummy, "log/%s.log", last_usr);

//		kdata_usr = fopen("log/kdata_usr.log", "r");		
//		FINAL_TEMPLATE = fopen(dummy, "a");
//		if(FINAL_TEMPLATE == NULL)	{	printf("Error. User log file was not found!");	return;	}		
//		compute_func2(FINAL_TEMPLATE);
//		while(LOCK == 1);		// While compute func 2 is working

////////////////////// Training phase *********** MOVED TO LOGIN ****** NEEDS TO BE DISCUSSED
//		gtk_label_set_text(status_box, "Training the network. Please wait. This may take a few minutes.");
//		create_merged_log_file();	printf("Merged log file...done.\n");
//		while(LOCK == 1);
//		create_list_of_deviations();	printf("Deviation file...done.\n");
//		while(LOCK);
//		create_deviations_final_file();

//		system("octave script.m");

		remove("log/kdata_usr.log");
		gtk_widget_destroy(window_register);
/*	}
	else
	{
		gtk_text_buffer_set_text(buf, "", sizeof(""));
		gtk_label_set_text(status_box, "Data not entered correctly. Please re-enter.");		
	}

printf("submit left...\n");
*/
//	gtk_main_quit();
}

void on_Show_Data_clicked(void)		// Need to change as per changed format of log files
{
	GtkEntry *username = (GtkEntry*)GTK_WIDGET(gtk_builder_get_object(builder_analysis, "Username_Entry2"));
	GtkEntry *password = (GtkEntry*)GTK_WIDGET(gtk_builder_get_object(builder_analysis, "Password_Entry2"));
	GtkLabel *label = (GtkLabel*)GTK_WIDGET(gtk_builder_get_object(builder_analysis, "UserDataLabel"));
	GtkTextView *maindata = (GtkTextView*)GTK_WIDGET(gtk_builder_get_object(builder_analysis, "MainData"));
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(maindata);

	FILE *temp;
	char buf[110];
	sprintf(buf, "log/%s.log", gtk_entry_get_text(username));
	if(gtk_entry_get_text_length(username) < 10 || gtk_entry_get_text_length(password) < 10)
	{	gtk_label_set_text(label, "Invalid username/password length.");	
		gtk_editable_delete_text((GtkEditable*)username, 0, -1);
		gtk_editable_delete_text((GtkEditable*)password, 0, -1);
	}
	else if((temp = fopen(buf, "r")) == NULL)
	{
		gtk_label_set_text(label, "Username entered is not registered.");
		gtk_widget_show((GtkWidget*)label);
		gtk_editable_delete_text((GtkEditable*)username, 0, -1);
		gtk_editable_delete_text((GtkEditable*)password, 0, -1);
	}
	else
	{
		char pass[100];
		fscanf(temp, "%s %s", pass, pass);
		if(strcmp(pass, gtk_entry_get_text(password)) == 0)
		{
			int i, c = linecount(temp);
			// Needs to change with changing format of "username.log"
			fscanf(temp, "%s %s", pass, pass);
			in = calloc(c-1, sizeof(struct template_input));

			for(i=0;i<c-1;i++)
				fscanf(temp, "%lf %lf %lf %lf %lf", &in[i].MHT_U, &in[i].MLT_U, &in[i].MHT_P, &in[i].MLT_P, &in[i].ATS);

			double _ATS = 0, _MHT = 0, _MLT = 0;
			for(i=0;i<c-1;i++) {
				_ATS = _ATS + in[i].ATS;
				_MHT = _MHT + in[i].MHT_U + in[i].MHT_P;
				_MLT = _MLT + in[i].MLT_U + in[i].MLT_P;
			}
			_ATS = _ATS/(double)(c-1);
			_MHT = _MHT/(double)(2*(c-1));
			_MLT = _MLT/(double)(2*(c-1));
			
			char b[200];
			sprintf(b, "Data for user %s:", gtk_entry_get_text(username));
			gtk_label_set_text(label, b);
			sprintf(b, "\nAverage Typing Speed (ATS) is %.4lf keys/sec\nAverage Hold Time is %.4lf ms\nAverage Lag Time is %.4lf ms", _ATS, _MHT, _MLT);
			gtk_text_buffer_set_text(buffer, b, strlen(b));
			gtk_widget_show((GtkWidget*)maindata);
		}
		else
		{
			gtk_label_set_text(label, "Wrong password! Enter again.");
			gtk_widget_show((GtkWidget*)label);
			gtk_editable_delete_text((GtkEditable*)password, 0, -1);
		}
	}
}

G_MODULE_EXPORT gboolean on_Large_Entry1_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey eventkey = event->key;
	GtkTextView *large = (GtkTextView*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Large_Entry1"));
	GtkButton *b = (GtkButton*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Submit_Button_R"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval) || eventkey.keyval == GDK_underscore || eventkey.keyval == GDK_space)
	{
//		GtkTextView *usrbuf = gtk_entry_get_buffer(large);
		gchar ch[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(ch, "_");
		else if(eventkey.keyval == GDK_space)
			strcpy(ch, " ");
		else	strcpy(ch, gdk_keyval_name(eventkey.keyval));

		// CODE to output keystroke data into file kdata_usr
		kdata_usr = fopen("log/kdata_usr.log", "a");
		fprintf(kdata_usr, "press %s %u %d %d\n", ch, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_usr);
	}
	else if(eventkey.keyval == GDK_BackSpace || eventkey.keyval == GDK_Delete)	// If backspace or delete os pressed, reset fields and empty datalog files
	{
		GtkTextBuffer * buf = gtk_text_view_get_buffer(large);
		gtk_text_buffer_set_text(buf, "", 0);
		gtk_text_view_set_buffer(large, buf);

//		gtk_entry_set_text(large, "");
		remove("log/kdata_usr.log");
		kdata_usr = fopen("log/kdata_usr.log", "a");
	}
	else if(eventkey.keyval == GDK_Return || eventkey.keyval == GDK_Tab)
		gtk_widget_grab_focus((GtkWidget*) b);		
	else ;	// Any other character entered entered in username field, do nothing
	{
//		gtk_label_set_text(status_box, "Invalid character for username. Please enter again.");
		// Enter code to reset data previously captured before invalid character and reset username field to empty
	}
}

gboolean on_Large_Entry1_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey eventkey = event->key;
	GtkTextView *large = (GtkTextView*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Large_Entry1"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval) || eventkey.keyval == GDK_underscore || eventkey.keyval == GDK_space)
	{
		gchar chr[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(chr, "_");
		else if(eventkey.keyval == GDK_space)
			strcpy(chr, " ");
		else	strcpy(chr, gdk_keyval_name(eventkey.keyval));

		kdata_usr = fopen("log/kdata_usr.log", "a");
		fprintf(kdata_usr, "release %s %u %d %d\n", chr, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_usr);
	}
	else ;	// Any other character entered entered in username field, do nothing
}

gboolean on_Login_Large_Entry_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey eventkey = event->key;
	GtkTextView *large = (GtkTextView*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Login_Large_Entry"));
	GtkButton *b = (GtkButton*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Authenticate_Button"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval) || eventkey.keyval == GDK_underscore || eventkey.keyval == GDK_space)
	{
//		GtkTextView *usrbuf = gtk_entry_get_buffer(large);
		gchar ch[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(ch, "_");
		else if(eventkey.keyval == GDK_space)
			strcpy(ch, " ");
		else	strcpy(ch, gdk_keyval_name(eventkey.keyval));

		// CODE to output keystroke data into file kdata_usr
		kdata_usr = fopen("log/kdata_usr.log", "a");
		fprintf(kdata_usr, "press %s %u %d %d\n", ch, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_usr);
	}
	else if(eventkey.keyval == GDK_BackSpace || eventkey.keyval == GDK_Delete)	// If backspace or delete os pressed, reset fields and empty datalog files
	{
		GtkTextBuffer * buf = gtk_text_view_get_buffer(large);
		gtk_text_buffer_set_text(buf, "", 0);
		gtk_text_view_set_buffer(large, buf);

//		gtk_entry_set_text(large, "");
		remove("log/kdata_usr.log");
		kdata_usr = fopen("log/kdata_usr.log", "a");
	}
	else if(eventkey.keyval == GDK_Return || eventkey.keyval == GDK_Tab)
		gtk_widget_grab_focus((GtkWidget*) b);		
	else ;	// Any other character entered entered in username field, do nothing
	{
//		gtk_label_set_text(status_box, "Invalid character for username. Please enter again.");
		// Enter code to reset data previously captured before invalid character and reset username field to empty
	}
}

gboolean on_Login_Large_Entry_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey eventkey = event->key;
	GtkTextView *large = (GtkTextView*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Login_Large_Entry"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval) || eventkey.keyval == GDK_underscore || eventkey.keyval == GDK_space)
	{
		gchar chr[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(chr, "_");
		else if(eventkey.keyval == GDK_space)
			strcpy(chr, " ");
		else	strcpy(chr, gdk_keyval_name(eventkey.keyval));

		kdata_usr = fopen("log/kdata_usr.log", "a");
		fprintf(kdata_usr, "release %s %u %d %d\n", chr, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_usr);
	}
	else ;	// Any other character entered entered in username field, do nothing
}

gboolean keyval_is_digit(guint keyval)		// Check if keyval corresponds to a digit
{
	if(keyval == GDK_0 || keyval == GDK_1 || keyval == GDK_2 || keyval == GDK_3 || keyval == GDK_4 || keyval == GDK_4 || keyval == GDK_5 || keyval == GDK_6 || keyval == GDK_7 || keyval == GDK_8 || keyval == GDK_9)	return TRUE;
	else	return FALSE;
}

gboolean keyval_is_alpha(guint keyval)		// Check is key keyval corresponds to an alphabet
{
	if((keyval >= GDK_A && keyval <= GDK_Z) || (keyval >= GDK_a && keyval <= GDK_z))
		return TRUE;
	else	return FALSE;
}

gboolean on_Username_Entry1_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey eventkey = event->key;
	GtkEntry *username = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Username_Entry1"));
	GtkLabel *status_box = (GtkLabel*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Status_Display"));
	GtkEntry *password = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Password_Entry1"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval) || eventkey.keyval == GDK_underscore)
	{
		GtkEntryBuffer *usrbuf = gtk_entry_get_buffer(username);
		gchar ch[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(ch, "_");
		else	strcpy(ch, gdk_keyval_name(eventkey.keyval));
/*
printf("ch = %s ", ch);
//		gint pos = gtk_editable_get_position((GtkEditable*)username);
		gint pos = gtk_entry_get_text_length(username);
printf("pos before = %d ", pos);
//		gtk_editable_set_position((GtkEditable*)username, -1);
//		gtk_editable_insert_text((GtkEditable*)username, ch, -1, &pos);
//		gtk_entry_buffer_insert_text(usrbuf, pos, ch, -1);
//		gtk_entry_set_buffer(username, usrbuf);
printf("pos after = %d\n", pos);
//		gtk_entry_append_text(username, ch);
*/
		// CODE to output keystroke data into file kdata_usr
		kdata_usr = fopen("log/kdata_usr.log", "a");
		fprintf(kdata_usr, "press %s %u %d %d\n", ch, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_usr);
	}
	else if(eventkey.keyval == GDK_BackSpace || eventkey.keyval == GDK_Delete)	// If backspace or delete os pressed, reset fields and empty datalog files
	{
		gtk_entry_set_text(username, "");
		remove("log/kdata_usr.log");
		kdata_usr = fopen("log/kdata_usr.log", "a");
	}
	else if(eventkey.keyval == GDK_Return || eventkey.keyval == GDK_Tab)
		gtk_widget_grab_focus((GtkWidget*) password);		
	else ;	// Any other character entered entered in username field, do nothing
	{
//		gtk_label_set_text(status_box, "Invalid character for username. Please enter again.");
		// Enter code to reset data previously captured before invalid character and reset username field to empty
	}
}

gboolean on_Username_Entry1_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey eventkey = event->key;
	GtkEntry *username = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Username_Entry1"));
	GtkLabel *status_box = (GtkLabel*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Status_Display"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval) || eventkey.keyval == GDK_underscore)
	{
		gchar chr[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(chr, "_");
		else	strcpy(chr, gdk_keyval_name(eventkey.keyval));

		kdata_usr = fopen("log/kdata_usr.log", "a");
		fprintf(kdata_usr, "release %s %u %d %d\n", chr, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_usr);
	}
	else ;	// Any other character entered entered in username field, do nothing
}

gboolean on_Password_Entry1_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{	// BUG:	Apparently accpets SHIFT+any_digit_key press. Should be handled.

	GdkEventKey eventkey = event->key;
	GtkEntry *password = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Password_Entry1"));
	GtkEntry *username = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Username_Entry1"));
	GtkLabel *status_box = (GtkLabel*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Status_Display"));
	GtkButton *s = (GtkButton*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Submit_Button_Register"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval))
	{
		gchar ch[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(ch, "_");
		else	strcpy(ch, gdk_keyval_name(eventkey.keyval));
/*
		gchar t[2];
		strcpy(t, "*");
//		gint pos = gtk_editable_get_position((GtkEditable*)username);
		gint pos = gtk_entry_get_text_length(password);
		gtk_editable_insert_text((GtkEditable*)password, ch, strlen(ch), &pos);
*/
		// CODE to output keystroke data into file
		kdata_pass = fopen("log/kdata_pass.log", "a");
		fprintf(kdata_pass, "press %s %u %d %d\n", ch, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_pass);
	}
	else if(eventkey.keyval == GDK_BackSpace || eventkey.keyval == GDK_Delete)	// If backspace or delete os pressed, reset fields and empty datalog files
	{
		gtk_entry_set_text(password, "");
		remove("kdata_pass.log");
		kdata_usr = fopen("log/kdata_pass.log", "a");
	}
	else if(eventkey.keyval == GDK_Tab || eventkey.keyval == GDK_Down)
	{
		gtk_widget_grab_focus((GtkWidget*) s);
	}
	else if(eventkey.keyval == GDK_Return)	// Mimic pressing of the submit button
		on_Submit_Button_Register_clicked();
	else ;	// Any other character entered entered in username field, do nothing
}

gboolean on_Password_Entry1_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey eventkey = event->key;
	GtkEntry *password = (GtkEntry*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Password_Entry1"));
	GtkLabel *status_box = (GtkLabel*) GTK_WIDGET(gtk_builder_get_object(builder_register, "Status_Display"));

	if(keyval_is_alpha(eventkey.keyval) || keyval_is_digit(eventkey.keyval))
	{
		gchar ch[2];
		if(eventkey.keyval == GDK_underscore)
			strcpy(ch, "_");
		else	strcpy(ch, gdk_keyval_name(eventkey.keyval));

		kdata_pass = fopen("log/kdata_pass.log", "a");
		fprintf(kdata_pass, "release %s %u %d %d\n", ch, eventkey.time, eventkey.state&1, eventkey.state&2);
		fclose(kdata_pass);
	}
	else ;	// Any other character entered entered in username field, do nothing	
}

void compute_func(FILE *F)
{	// This function will read raw keystroke data from kdata_usr and kdata_pass files, calculate necessary data and put them in FINAL_TEMPLATE file.
	// Note: All file pointers are globally accessible and all necessary files have already been opened by calling functions
	LOCK = 1;
printf("Entering compute...\n");
	guint begin, end;		// Used to calculate average typing speed
	guint cumulative = 0;		// Used to calculate average typing speed : Cumulative time for usr and pass
	int	totalkeys = 0;		// Used to calculate average typing speed: Total keys pressed

	int i, j, k = linecount(kdata_usr);	//printf("usr linecount = %d\n", k);
	in_data = calloc(k, sizeof(struct input));
	for(i=0;i<k;i++) {
		fscanf(kdata_usr, "%s %c %d %d %d", in_data[i].type, &(in_data[i].key), &(in_data[i].time), &(in_data[i].shift), &(in_data[i].caps));	
//		printf("%s %c %d %d %d\n", in_data[i].type, (in_data[i].key), (in_data[i].time), (in_data[i].shift), (in_data[i].caps));
	}

	begin = in_data[0].time;	// Assuming that always in_data[0] refers to the first press and that 
	end = in_data[k-1].time;	// in_data[k-1] is the last release. May not be always true. # SUBJECT TO CHANGE

	guint last_release = 0, lag_time = 0;
	double hold_time = 0;
	double mean_hold, mean_lag, avg_speed;
	int key_count = k/2;
	// Avg. speed calculation
	if(end >= begin)	cumulative = cumulative + end-begin;
	else			cumulative = cumulative + UINT_MAX - begin + end;
	totalkeys = totalkeys + key_count;

	double USR_HOLD_TIME[k];
	int hc = 0;			// Hold Count

	for(i=0;i<k;i++)
		if(strcmp(in_data[i].type, "press") == 0)
		{
			if(last_release != 0)
				lag_time = lag_time + (guint)fabs(in_data[i].time - last_release);	// fabs always keeps it positive

			for(j=i; j<k;j++)
				if(strcmp(in_data[j].type, "release")==0 && in_data[i].key == in_data[j].key)
				{	
					USR_HOLD_TIME[hc++] = fabs(in_data[j].time - in_data[i].time);
					hold_time = hold_time + USR_HOLD_TIME[hc-1];
					last_release = in_data[j].time;
					break;
				}
		}

	mean_hold = (double)hold_time/(double)key_count;
	mean_lag = (double)lag_time/(double)key_count;

	fprintf(F, "%lf %lf ", scale(mean_hold), scale(mean_lag));
	free(in_data);

	k = linecount(kdata_pass);	//printf("pass linecount = %d\n", k);
	in_data = calloc(k, sizeof(struct input));
	for(i=0;i<k;i++)
		fscanf(kdata_pass, "%s %c %d %d %d", in_data[i].type, &(in_data[i].key), &(in_data[i].time), &(in_data[i].shift), &(in_data[i].caps));
	
	double PASS_HOLD_TIME[k];
	last_release = 0, hold_time = 0, lag_time = 0;
//	double mean_hold, mean_lag;
	key_count = k/2;

	// Avg. speed calculation
	if(end >= begin)	cumulative = cumulative + end-begin;
	else			cumulative = cumulative + UINT_MAX - begin + end;
	totalkeys = totalkeys + key_count;

	hc = 0;	
	for(i=0;i<k;i++)
		if(strcmp(in_data[i].type, "press") == 0)
		{
			if(last_release != 0)
				lag_time = lag_time + (guint)fabs(in_data[i].time - last_release);	// fabs always keeps it positive

			for(j=i; j<k;j++)
				if(strcmp(in_data[j].type, "release")==0 && in_data[i].key == in_data[j].key)
				{
					PASS_HOLD_TIME[hc++] = fabs(in_data[j].time - in_data[i].time);
					hold_time = hold_time + PASS_HOLD_TIME[hc-1];
					last_release = in_data[j].time;
					break;
				}
		}

	mean_hold = (double)hold_time/(double)key_count;
	mean_lag = (double)lag_time/(double)key_count;

	// Hold time variance calculation
	double MEAN = 0;
	for(i=0;i<k;i++)
		MEAN = MEAN + USR_HOLD_TIME[i] + PASS_HOLD_TIME[i];
	MEAN = MEAN/(double)(2*k);

	double VAR = 0, SD;
	for(i=0;i<k;i++)
		VAR = VAR + USR_HOLD_TIME[i]*USR_HOLD_TIME[i] + PASS_HOLD_TIME[i]*PASS_HOLD_TIME[i];
	VAR = VAR/(double)(2*k);
	
	VAR = VAR - MEAN;
	SD = sqrt(VAR);
		
	avg_speed = (double)(totalkeys * 1000)/(double)(cumulative);
	fprintf(F, "%lf %lf %lf %lf ", scale(mean_hold), scale(mean_lag), scale(avg_speed), scale(SD));	// Final avg_speed is in keys pressed per second
	free(in_data);

	rewind(kdata_usr);
	rewind(kdata_pass);
	double GF_usr = calculate_GF(kdata_usr);
	double GF_pass = calculate_GF(kdata_pass);
	double avg_GF;		// avg_GF(Average Grouping Factor) is calculated as:
				//	If s1 is Standard Deviation (S.D.) of username calculated from kdata_usr	
				//	and s2 is S.D. of password calculated from kdata_pass
				//	Then suppose f1 = s1/(s1+s2)	and f2 = s2/(s1+s2)	variance fractions
				//	then avg_GF = f1*GF_pass + f2*GF_usr
				// 	Reason is, Higher S.D. should get lesser weight in this calculation.

//	Release this after writing S.D. computing function in "auxiliary.h"

	rewind(kdata_usr);
	rewind(kdata_pass);
//	double s1 = SD(kdata_usr);
//	double s2 = SD(kdata_pass);
//	avg_GF = (s1*GF_pass + s2*GF_usr)/(s1+s2);
	avg_GF = (GF_pass + GF_usr)/(double)2;
	fprintf(F, "%lf\n", scale(avg_GF));
//fprintf(F, "\n");	// GF removed

	// Show brief analysis in Status Box
	GtkLabel *stl = (GtkLabel *)GTK_WIDGET(gtk_builder_get_object(builder_register, "Status_Display"));
	char tbuf[200];
	sprintf(tbuf, "Average Dwell Time\t    %lf ms\nAvg. Inter-key Time\t    %lf ms\nAverage Speed\t    %lf keys/sec\nAvg. Grouping Factor\t	%lf", mean_hold, mean_lag, avg_speed, avg_GF);
	gtk_label_set_text(stl, tbuf);
printf("Leaving compute...\n");
	LOCK = 0;
}

void compute_func2(FILE *F)
{printf("Entering compute 2...\n");
	LOCK = 1;		// Lock set
guint begin, end;		// Used to calculate average typing speed
		guint cumulative = 0;		// Used to calculate average typing speed : Cumulative time for usr and pass
		int	totalkeys = 0;		// Used to calculate average typing speed: Total keys pressed
	
		int i, j, k = linecount(kdata_usr);	//printf("usr linecount = %d\n", k);
		in_data = calloc(k, sizeof(struct input));
		for(i=0;i<k;i++) {
			fscanf(kdata_usr, "%s %c %d %d %d", in_data[i].type, &(in_data[i].key), &(in_data[i].time), &(in_data[i].shift), &(in_data[i].caps));	
	//		printf("%s %c %d %d %d\n", in_data[i].type, (in_data[i].key), (in_data[i].time), (in_data[i].shift), (in_data[i].caps));
		}
	
		begin = in_data[0].time;	// Assuming that always in_data[0] refers to the first press and that 
		end = in_data[k-1].time;	// in_data[k-1] is the last release. May not be always true. # SUBJECT TO CHANGE
	
		guint last_release = 0, lag_time = 0;
		double hold_time = 0;
		double mean_hold, mean_lag, avg_speed;
		int key_count = k/2;
		// Avg. speed calculation
		if(end >= begin)	cumulative = cumulative + end-begin;
		else			cumulative = cumulative + UINT_MAX - begin + end;
		totalkeys = totalkeys + key_count;

		double USR_HOLD_TIME[k];
		int hc = 0;			// Hold Count
	
		for(i=0;i<k;i++)
			if(strcmp(in_data[i].type, "press") == 0)
			{
				if(last_release != 0)
					lag_time = lag_time + (guint)fabs(in_data[i].time - last_release);	// fabs always keeps it positive

				for(j=i; j<k;j++)
					if(strcmp(in_data[j].type, "release")==0 && in_data[i].key == in_data[j].key)
					{	
						USR_HOLD_TIME[hc++] = fabs(in_data[j].time - in_data[i].time);
						hold_time = hold_time + USR_HOLD_TIME[hc-1];
						last_release = in_data[j].time;
						break;
					}
			}

		mean_hold = (double)hold_time/(double)key_count;
		mean_lag = (double)lag_time/(double)key_count;
	
		fprintf(F, "%lf %lf ", scale(mean_hold), scale(mean_lag));
		free(in_data);
		double MEAN = 0;
		for(i=0;i<k;i++)
			MEAN = MEAN + USR_HOLD_TIME[i];
		MEAN = MEAN/(double)(k);
	
		double VAR = 0, SD;
		for(i=0;i<k;i++)
			VAR = VAR + USR_HOLD_TIME[i]*USR_HOLD_TIME[i];
		VAR = VAR/(double)(k);
		
		VAR = VAR - MEAN;
		SD = sqrt(VAR);
			
		avg_speed = (double)(totalkeys * 1000)/(double)(cumulative);
		fprintf(F, "%lf %lf %lf %lf ", scale(mean_hold), scale(mean_lag), scale(avg_speed), scale(SD));	// Final avg_speed is in keys pressed per second
		double GF_usr = calculate_GF(kdata_usr);
		fprintf(F, "%lf\n", scale(GF_usr));
//fprintf(F, "\n");		// GF removed
		fclose(F);

		LOCK = 0;		// Lock released
printf("Leaving compute 2...\n");
}

void on_Authenticate_Button_clicked(void)
{	// Remember to set Login_Complete == TRUE if everything is correct, else if not aythenticated remove temp files
	// Run Octave scritp/C script for BPN
//	system("octave script.m");
	char text1[] = "The quick brown fox jumps over the lazy dog";
	char text2[] = "the quick brown fox jumps over the lazy dog";

	GtkLabel *label = (GtkLabel *)GTK_WIDGET(gtk_builder_get_object(builder_login, "Login_Status"));
	GtkTextView *Large_Text = (GtkTextView*) GTK_WIDGET(gtk_builder_get_object(builder_login, "Login_Large_Entry"));
	GtkTextIter start, end;
	GtkTextBuffer *buf = gtk_text_view_get_buffer(Large_Text);
	gtk_text_buffer_get_iter_at_offset(buf, &start, 0);
	gtk_text_buffer_get_iter_at_offset(buf, &end, -1);
	char *text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
//	if(strcmp(text, text1)==0 || strcmp(text, text2)==0)
//	{	// String entered correctly
		// ___Train with deviations_final_file
		create_merged_log_file();	printf("Merged log file...done.\n");
		while(LOCK == 1);
		create_list_of_deviations();	printf("Deviation file...done.\n");
//		create_deviations_final_file(login_attempt_usr);
//		system("octave script.m");

		TEMPLOGIN = fopen("tmp/templogin", "a");
		kdata_usr = fopen("log/kdata_usr.log", "r");
		
//		compute_func2(TEMPLOGIN);
		while(LOCK == 1) printf("Locked");

//		fclose(TEMPLOGIN);
		TEMPLOGIN = fopen("tmp/templogin", "r");

//		char name[120];
//		sprintf(name, "log/%s.log", login_attempt_usr);
//		FILE *temp = fopen(name, "r");
//		if(temp == NULL)	{	printf("Error. Data file not found during authentication.\n");	exit(EXIT_FAILURE);	}
		
		// Neural Network Analysis/Recognition phase
		// The function feed_into_network(FILE *F) takes a file F containing deviation values of the attempting user
		// passes the values of the file into the NN, and returns output of the network
		// Based on these output, decisions are taken
		// The fed file F is always assumed to have 2 lines of values
		// 1st line : contains the deviation values calculated for the first line in TEMPLOGIN, i.e. the deviation values for username/password entered
		// 2nd line : contains the deviation values calculated for the second line in TEMPLOGIN i.e. the deviation values for the text entered.
printf("Stage1\n");
		remove("tmp/merged_log_file");		// Remove merged_log_file, if any exists
		create_merged_log_file();
		FILE *m = fopen("tmp/merged_log_file", "r");
		int i, j, N;	
		char username[120], dummy[120];
		struct template_input read, mean, dev;
printf("Stage2\n");
		fscanf(m, "%d", &N);
		FILE *feed = fopen("tmp/feedfile", "w");

		for(i=0;i<N;i++)
		{printf("Stage3\n");
			int k;
			fscanf(m, "%s %s %s %s", username, username, dummy, dummy);
			fscanf(m, "%d", &k);
			for(j=0;j<k;j++)
				fscanf(m, "%lf %lf %lf %lf %lf %lf %lf", &mean.MHT_U, &mean.MLT_U, &mean.MHT_P, &mean.MLT_P, &mean.ATS, &mean.SD, &mean.GF);
			fscanf(m, "%lf %lf %lf %lf %lf %lf %lf", &mean.MHT_U, &mean.MLT_U, &mean.MHT_P, &mean.MLT_P, &mean.ATS, &mean.SD, &mean.GF);
			
			if(strcmp(username, login_attempt_usr) == 0)
			{
				fscanf(TEMPLOGIN, "%lf %lf %lf %lf %lf %lf %lf", &read.MHT_U, &read.MLT_U, &read.MHT_P, &read.MLT_P, &read.ATS, &read.SD, &read.GF);
//				dev.MHT_U = read.MHT_U - mean.MHT_U;
//				dev.MLT_U = read.MLT_U - mean.MLT_U;
//				dev.MHT_P = read.MHT_P - mean.MHT_P;
//				dev.MLT_P = read.MLT_P - mean.MLT_P;
//				dev.ATS = read.ATS - mean.ATS;
//				dev.SD = read.SD - mean.SD;
//				dev.GF = read.GF - mean.GF;
				dev.MHT_U = read.MHT_U;
				dev.MLT_U = read.MLT_U;
				dev.MHT_P = read.MHT_P;
				dev.MLT_P = read.MLT_P;
				dev.ATS = read.ATS;
				dev.SD = read.SD;
				dev.GF = read.GF;
printf("Print_logi output:\n%lf %lf %lf %lf %lf %lf %lf\n", scale(dev.MHT_U), scale(dev.MLT_U), scale(dev.MHT_P), scale(dev.MLT_P), scale(dev.ATS), scale(dev.SD), scale(dev.GF));
				fprintf(feed, "%lf %lf %lf %lf %lf %lf %lf\n", scale(dev.MHT_U), scale(dev.MLT_U), scale(dev.MHT_P), scale(dev.MLT_P), scale(dev.ATS), scale(dev.SD), scale(dev.GF));

//				fscanf(TEMPLOGIN, "%lf %lf %lf %lf %lf %lf %lf", &read.MHT_U, &read.MLT_U, &read.MHT_P, &read.MLT_P, &read.ATS, &read.SD, &read.GF);
//				dev.MHT_U = read.MHT_U - mean.MHT_U;
//				dev.MLT_U = read.MLT_U - mean.MLT_U;
//				dev.MHT_P = read.MHT_P - mean.MHT_P;
//				dev.MLT_P = read.MLT_P - mean.MLT_P;
//				dev.ATS = read.ATS - mean.ATS;
//				dev.SD = read.SD - mean.SD;
//				dev.GF = read.GF - mean.GF;
/*				dev.MHT_U = read.MHT_U;
				dev.MLT_U = read.MLT_U;
				dev.MHT_P = read.MHT_P;
				dev.MLT_P = read.MLT_P;
				dev.ATS = read.ATS;
				dev.SD = read.SD;
				dev.GF = read.GF;
*/
//				fprintf(feed, "%lf %lf %lf %lf %lf %lf %lf\n", scale(dev.MHT_U), scale(dev.MLT_U), scale(dev.MHT_P), scale(dev.MLT_P), scale(dev.ATS), scale(dev.SD), scale(dev.GF));
				
				break;
			}
		}
		fclose(feed);
//		feed = fopen("tmp/feedfile", "r");
//		double *val = feed_into_network(feed);
//		printf("Values returned: %lf %lf\n", val[0], val[1]);

		// Start KNN  ----------------------->>>>
// DO NOT UNCOMMENT  -----<>
//		int *KNNVAL;
//		KNNVAL = get_KNNVAL();

		TEMPLOGIN = fopen("tmp/templogin", "r");
		rewind(m);
		FILE *refresh = fopen("tmp/KNN_RETURN", "w");
		fclose(refresh);
// Multiple-run KNN		
// KNNVAL above 10 and 20 will be majorly flawed since we only want it to match a particular user, for whom there can be a maximum of 10 matches.
// Hence KNNVAL above 20 may cause problems. In this case, get_KNNVAL() is not necessary.
// Also in this case, using 10, 15 'AND' 20 will be redundant, since the larger KNNVAL covers the smaller ones.

//printf("KNNVAL = %d\n", KNNVAL[0]);
//		KNN(TEMPLOGIN, m, 10);
//printf("KNNVAL = %d\n", KNNVAL[1]);
//		KNN(TEMPLOGIN, m, 15);
//printf("KNNVAL = %d\n", KNNVAL[2]);

		KNN(TEMPLOGIN, m, KNNVAL);

//////////// HERE 1
		int q;
		struct registered_users *reg = get_registered_usernames();

//		for(q=0;q<reg->N;q++)
//			printf("%s\n", reg->name[q]);
/////
		int count[reg->N];
		double total_dist[reg->N];
		int iter, p;
		for(iter=0; iter<reg->N; iter++)
		{	count[iter] = 0;	total_dist[iter] = 0;	}

		FILE *knn_out = fopen("tmp/KNN_RETURN", "r");
		int ln = linecount(knn_out);
		rewind(knn_out);
		char **names;
		names = calloc(ln, sizeof(char *));
		for(iter = 0; iter < ln; iter++)
			names[iter] = calloc(120, sizeof(char));
		double dist[ln];
		for(iter = 0;iter<ln;iter++)
			fscanf(knn_out, "%s %lf", names[iter], &dist[iter]);

		fclose(knn_out);

//for(iter = 0; iter < ln; iter++)
//	printf("%s %lf\n", names[iter], dist[iter]);

		for(iter = 0;iter < ln;iter++) {
			for(p=0;p < reg->N; p++)
			{	if(strcmp(names[iter], reg->name[p]) == 0)
				{	count[p]++;	total_dist[p] += dist[iter];
					if(count[p] >= 3)	printf("KNN definitive match for: %s\n", reg->name[p]);		// Note: 3 is the acceptance-threshold
				}	}
		}
//////////// FOR DEBUGGING *********
// Print counter
printf("Printing user counts...\nName  Count  TD\n");
for(iter = 0; iter<reg->N; iter++)
	printf("%s %d %lf\n", reg->name[iter], count[iter], total_dist[iter]);
///////////////// ******************

	// To find out the top 3 awesomest matches, and write to file

		FILE *topusers = fopen("tmp/top_users", "w");	// Output will be written to this file

		int max1 = 0, max2 = 0, max3 = 0, check = 0;
		for(iter = 0;iter < reg->N;iter++)
			if(count[iter] > max1)	max1 = count[iter];	// max1 is the highest occuring count for any user(s)
printf("max1 = %d\n", max1);
		for(iter = 0; iter < reg->N; iter++)
			if(count[iter] > max2 && count[iter] < max1)	max2 = count[iter];
printf("max2 = %d\n", max2); 
		for(iter = 0; iter < reg->N; iter++)
			if(count[iter] > max3 && count[iter] < max2) 	max3 = count[iter];
printf("max3 = %d\n", max3);
		int no_of_max = 0, auxc = 0;
		for(iter = 0;iter < reg->N;iter++)
			if(count[iter] == max1)	no_of_max++;
printf("no_of_max = %d\n", no_of_max);

				char max2names[auxc][120];
				double max2dist[auxc];
				int tracker =0;
				double mindist1 = INFINITE,
					mindist2 = INFINITE;

		if(no_of_max >= 3)
		{// In this case we have top 3 matching users. just need to order them by distance
			char topname[no_of_max][120];
			int track = 0;
			double topdist[no_of_max];
			for(iter = 0; iter < reg->N; iter++)
				if(count[iter] == max1)	{	strcpy(topname[track], reg->name[iter]);	topdist[track] = total_dist[iter];	}
			int dmin1 = INFINITE, dmin2 = INFINITE, dmin3 = INFINITE;
			for(iter = 0; iter < no_of_max; iter++)
				if(topdist[iter] < dmin1)	dmin1 = topdist[iter];
			for(iter = 0; iter < no_of_max; iter++)
				if(topdist[iter] < dmin2 && topdist[iter] > dmin1)	dmin2 = topdist[iter];
			for(iter = 0; iter < no_of_max; iter++)
				if(topdist[iter] < dmin3 && topdist[iter] > dmin2)	dmin3 = topdist[iter];
			for(iter = 0; iter < no_of_max; iter++)
				if(topdist[iter] == dmin1 && topname[iter]!=" ")	fprintf(topusers, "%s\n", topname[iter]);
			for(iter = 0; iter < no_of_max; iter++)
				if(topdist[iter] == dmin2 && topname[iter]!=" ")	fprintf(topusers, "%s\n", topname[iter]);
			for(iter = 0; iter < no_of_max; iter++)
				if(topdist[iter] == dmin3 && topname[iter]!=" ")	fprintf(topusers, "%s\n", topname[iter]);
		}
		else if(no_of_max == 2)
		{// In this case we have top 2 matching users. Just need to order them by distance and find the 3rd top match.
//			printf("TODO Code!\n");
			char max1names[2][120];
			double max1dist[2];
			int trk = 0;
			for(iter = 0; iter < reg->N; iter++)
				if(count[iter] == max1)	{	strcpy(max1names[trk], reg->name[iter]);	max1dist[trk] = total_dist[iter];	trk++;	}
			if(max1dist[0] < max1dist[1])
				fprintf(topusers, "1 %s\n2 %s\n", max1names[0], max1names[1]);
			else	fprintf(topusers, "1 %s\n2 %s\n", max1names[1], max1names[0]);

			// Get 3rd best
			int no_of_max2 = 0;
			for(iter = 0; iter < reg->N; iter++)
				if(count[iter] == max2)	no_of_max2++;
				char max2n[no_of_max2][120];
				double max2d[no_of_max2];
				int tracer = 0;
				int min_dist = INFINITE;

			if(no_of_max2 == 1)
			{
				for(iter = 0; iter < reg->N; iter++)
					if(count[iter] == max2)	fprintf(topusers, "3 %s\n", reg->name[iter]);
			}
			else
			{
				for(iter = 0; iter < reg->N; iter++)
					if(count[iter] == max2)	{	strcpy(max2n[tracer], reg->name[iter]);	max2d[tracer] = total_dist[iter];	tracer++;		}
				for(iter = 0; iter < no_of_max2; iter++)
					if(max2d[iter] < min_dist)	min_dist = max2d[iter];
				for(iter = 0; iter < no_of_max2; iter++)
					if(min_dist == max2d[iter])	{	fprintf(topusers, "3 %s\n", max2n[iter]);	break;	}
			}
		}
		else if(no_of_max == 1)
		{// In this case, we have only the top matching user. Need to find the next 2.
			for(iter = 0; iter < reg->N; iter++)
				if(count[iter] == max1)	fprintf(topusers, "1 %s\n", reg->name[iter]);
			auxc =0;
			for(iter = 0; iter < reg->N; iter++)
				if(count[iter] == max2)	auxc++;
printf("auxc = %d\n", auxc);
			if(auxc == 1)
			{	for(iter = 0; iter < reg->N; iter++)	if(count[iter] == max2)	fprintf(topusers, "2 %s\n", reg->name[iter]);
/*				int tc = 0;
				for(iter = 0; iter < reg->N; iter++)
					if(count[iter] == max3)	tc++;
					if(tc == 1)
					{	for(iter = 0; iter < reg->N; iter++)	if(count[iter] == max3)	fprintf(topusers, "3 %s\n", reg->name[iter]);	}
					else if(tc >= 2)
					{
					}			
*/
				for(iter = 0; iter < reg->N; iter++)	{	if(count[iter] == max3)	{fprintf(topusers, "3 %s\n", reg->name[iter]);	break;}	}
			}
			else if(auxc >= 2)
			{	printf("auxc = %d from inside\n", auxc);
				for(iter = 0; iter < reg->N; iter++)
					if(count[iter] == max2)	{	strcpy(max2names[tracker], reg->name[iter]);	max2dist[tracker] = total_dist[iter];	tracker++;	}

//for(iter = 0; iter < auxc; iter++)	printf("%s %lf\n", max2names[iter], max2dist[iter]);

				for(iter = 0; iter < auxc; iter++)
					if(max2dist[iter] < mindist1)	mindist1 = max2dist[iter];
//printf("mindist1 = %lf\n", mindist1);
				for(iter = 0; iter < auxc; iter++)
					if(max2dist[iter] < mindist2 && max2dist[iter] > mindist1)	mindist2 = max2dist[iter];
//printf("mindist2 = %lf\n", mindist2);
				for(iter = 0; iter < auxc; iter++)
					if(max2dist[iter] == mindist1)	fprintf(topusers, "2 %s\n", max2names[iter]);
				for(iter = 0; iter < auxc; iter++)
					if(max2dist[iter] == mindist2)	fprintf(topusers, "3 %s\n", max2names[iter]);					
			}
		}
		fclose(topusers);

////////////////	Create final deviations file as per the new rules. Running Neural Net...
	create_new_rule_devfile();
	while(LOCK);
	system("octave script.m");
		feed = fopen("tmp/feedfile", "r");
		double *val = feed_into_network(feed);
		printf("Values returned: %lf %lf\n", val[0], val[1]);
//////////////////
/*		for(iter = 0; iter < reg->N; iter++)
			if(count[iter] > max2 && count[iter] < max1) 

		for(iter = 0; iter < reg->N ; iter++)
			if(count[iter] == max)
				if(strcmp(login_attempt_usr, reg->name[iter]) == 0)
				{	check = 1;	printf("KNN: Match found: %s\n", reg->name[iter]);	break;	}

		if(check == 0)	printf("KNN: No match found.\n");

		knn_out = fopen("tmp/KNN_RETURN", "r");
		
		int lines = linecount(knn_out);
		rewind(knn_out);
		char out_name[120];
		check = 0;

		for(i=0;i<lines;i++)	{
			fscanf(knn_out, "%s", out_name);
			if(strcmp(login_attempt_usr, out_name) == 0)
			{	printf("KNN Match found.\n");	check=1;	break;	}
		}
		if(check == 0)	printf("KNN did not match.\n");

*/

//	}
//	else
//	{
//		gtk_text_buffer_set_text(buf, "", sizeof(""));
//		gtk_label_set_text(label, "Data not entered correctly. Please re-enter.");		
//	}


/*	struct registered_users *reg = get_registered_usernames();
	FILE *knn_out = fopen("tmp/KNN_RETURN", "r");

//	struct top_users instance;
//	instance = return_top_users(knn_out);

	int it, lines = linecount(knn_out);
	double cnt = 0;
	char read_name[100];
	double dist;
	for(it = 1; it <= lines; it++) {
		fscanf(knn_out, "%s %lf", read_name, &dist);
		if(strcmp(read_name, login_attempt_usr) == 0)	cnt = cnt + (double)1/(double)dist;	// 'cnt' IS INCREMENTED BASED ON THE DISTANCE OF A MATCH
	}							// NOTE IMPORTANT!!! Even if all first 10 values match for a particular user in two cases, the distance ranges in each case may be different leading to different 'cnt' values, so need to NORMALIZE 'cnt'
// One option is to do (after cnt = cnt + 1/dist): cnt = cnt * (first_match_dist + last_match_dist)	as below:

	rewind(knn_out);
	double first, last, temp;
	for(it=0;it<lines;it++) {
		fscanf(knn_out, "%s %lf", read_name, &first);
		if(strcmp(read_name, login_attempt_usr) == 0)	break;	}

	rewind(knn_out);
	for(it=0;it<lines;it++) {
		fscanf(knn_out, "%s %lf", read_name, &temp);
		if(strcmp(read_name, login_attempt_usr) == 0) last = temp;	}

	cnt = cnt * (first + last);

printf("KNN cnt val = %lf\n", cnt);	*/
//	double percentage = (double)cnt/(double)lines;
//	if( percentage > 0.5 )	printf("KNN Match found for %s. Certainty level: %lf\n", login_attempt_usr, percentage);
//	else			printf("KNN did not meet threshold. No certain match found.\n");

}

int linecount(FILE *f)
{//printf("in count\n");
	rewind(f);
	char buf[100];
	int count = 0;
	while(fgets(buf, 100, f) != NULL)
		count++;
	rewind(f);
//	printf("count = %d\n", count);
	return count;
}

// 	The next 5 functions are callbacks for Settings change events

G_MODULE_EXPORT void on_Highest_Level_clicked(void)
{
	FILE *temp = fopen("settings.conf", "w");
	fprintf(temp, "\\\\ Note: \"level\" can only be 1, 2, 3, 4 or 5.\nlevel = 5");	
	fclose(temp);
}

G_MODULE_EXPORT void on_High_Level_clicked(void)
{
	FILE *temp = fopen("settings.conf", "w");
	fprintf(temp, "\\\\ Note: \"level\" can only be 1, 2, 3, 4 or 5.\nlevel = 4");	
	fclose(temp);
}

G_MODULE_EXPORT void on_Medium_Level_clicked(void)
{
	FILE *temp = fopen("settings.conf", "w");
	fprintf(temp, "\\\\ Note: \"level\" can only be 1, 2, 3, 4 or 5.\nlevel = 3");	
	fclose(temp);
}

G_MODULE_EXPORT void on_Low_Level_clicked(void)
{
	FILE *temp = fopen("settings.conf", "w");
	fprintf(temp, "\\\\ Note: \"level\" can only be 1, 2, 3, 4 or 5.\nlevel = 2");	
	fclose(temp);
}

G_MODULE_EXPORT void on_Lowest_Level_clicked(void)
{
	FILE *temp = fopen("settings.conf", "w");
	fprintf(temp, "\\\\ Note: \"level\" can only be 1, 2, 3, 4 or 5.\nlevel = 1");	
	fclose(temp);
}

int get_usr_log_files(void)		// returns 1 if successful, 0 if no log files found
{
	system("ls log/ > tmp/templist");
	system("egrep .log tmp/templist > tmp/listtemp");
	FILE *t = fopen("tmp/listtemp", "r");
	FILE *list = fopen("tmp/list", "w");
	int i, n = linecount(t);
	if(n == 0)
		return 0;
	else
	{
		char buf[120];
		
		while(fscanf(t, "%s", buf) != EOF)
		{	if(strcmp(buf, "kdata_usr.log") != 0 && strcmp(buf, "kdata_pass.log") != 0 && strcmp(buf, "merged_log_file") != 0)
				fprintf(list, "%s\n", buf);
		}
	}
	remove("tmp/templist");
	remove("tmp/listtemp");
	fclose(list);

	return 1;
}

int create_merged_log_file(void)		// Merges all log files into one: "merged_log_file"
{printf("Enter merge_log...\n");
	LOCK = 1;		// Set lock
	if(get_usr_log_files() == 0)	{	remove("list");	return 0;	}
	FILE *list = fopen("tmp/list", "r");
	if(list == NULL)	{	printf("Error! No list file found in tmp directory.\n");	return 0;	}
	FILE *temp;
	FILE *out = fopen("tmp/merged_log_file", "w");
	int i, j, n = linecount(list);		// n = no. of log files found
	char l[n][120];
	char dummy[120];

	fprintf(out, "%d\n\n", n);		// First line of merged_log_file: n (no. of users merged in this file)

	for(i=0;i<n;i++)
	{
		fscanf(list, "%s", l[i]);
		char buff[120];
		sprintf(buff, "log/%s", l[i]);
		temp = fopen(buff, "r");
		int x = linecount(temp) - 1;

		struct template_input t[x];

//////////////////////////// Filtering out the .log part
		int k = 0;
		char ftmp[120];
		while(l[i][k] != '.')
		{	ftmp[k] = l[i][k];	k++;	}
		ftmp[k] = '\0';
///////////////////////////

		fscanf(temp, "%s %s", dummy, dummy);
		fprintf(out, "username %s\npassword %s\n%d\n", ftmp, dummy, x);
		x=LEVEL*2;
                for(j=0;j<x;j++) {
			fscanf(temp, "%lf %lf %lf %lf %lf %lf %lf", &t[j].MHT_U, &t[j].MLT_U, &t[j].MHT_P, &t[j].MLT_P, &t[j].ATS, &t[j].SD, &t[j].GF);
			fprintf(out, "%lf %lf %lf %lf %lf %lf %lf\n", scale(t[j].MHT_U), scale(t[j].MLT_U), scale(t[j].MHT_P), scale(t[j].MLT_P), scale(t[j].ATS), scale(t[j].SD), scale(t[j].GF));
		}
		struct template_input mean;
		mean.MHT_U = 0;
		mean.MLT_U = 0;
		mean.MHT_P = 0;
		mean.MLT_P = 0;
		mean.ATS = 0;
		mean.SD = 0;
		mean.GF = 0;

		for(j=0;j<x;j++)
		{
			mean.MHT_U = mean.MHT_U + t[j].MHT_U;
			mean.MLT_U = mean.MLT_U + t[j].MLT_U;
			mean.MHT_P = mean.MHT_P + t[j].MHT_P;
			mean.MLT_P = mean.MLT_P + t[j].MLT_P;
			mean.ATS = mean.ATS + t[j].ATS;
			mean.SD = mean.SD + t[j].SD;
			mean.GF = mean.GF + t[j].GF;
		}
		mean.MHT_U = mean.MHT_U/(double)x;
		mean.MLT_U = mean.MLT_U/(double)x;
		mean.MHT_P = mean.MHT_P/(double)x;
		mean.MLT_P = mean.MLT_P/(double)x;
		mean.ATS = mean.ATS/(double)x;
		mean.SD = mean.SD/(double)x;
		mean.GF = mean.GF/(double)x;

		fprintf(out, "\n%lf %lf %lf %lf %lf %lf %lf\n\n", scale(mean.MHT_U), scale(mean.MLT_U), scale(mean.MHT_P), scale(mean.MLT_P), scale(mean.ATS), scale(mean.SD), scale(mean.GF));
	}
	remove("tmp/list");
	fclose(out);
	LOCK = 0;			// Remove lock
	printf("Leave merge_log...\n");
	return 1;
}

int create_list_of_deviations(void)	// This function should always be called following a call to create_merged_log_file
{
	LOCK = 1;	// Lock
	FILE *merged = fopen("tmp/merged_log_file", "r");
	FILE *list = fopen("tmp/list_of_deviations", "a");
	if(merged == NULL)	{	printf("Error: merged_log_file not found.\n");	return 0;	}
	// If found
	int i, j, N_USERS;
	char dummy[120];
	fscanf(merged, "%d", &N_USERS);

	for(i=0;i<N_USERS;i++)
	{
		char current_usr[120];
		fscanf(merged, "%s %s %s %s", dummy, current_usr, dummy, dummy);	// Read username, <username>, password, <password> and ignore
		int m;
		fscanf(merged, "%d", &m);
		struct template_input t[m];
		struct template_input mean, dev;

		for(j=0;j<m;j++)
			fscanf(merged, "%lf %lf %lf %lf %lf %lf %lf", &t[j].MHT_U, &t[j].MLT_U, &t[j].MHT_P, &t[j].MLT_P, &t[j].ATS, &t[j].SD, &t[j].GF);

		fscanf(merged, "%lf %lf %lf %lf %lf %lf %lf", &mean.MHT_U, &mean.MLT_U, &mean.MHT_P, &mean.MLT_P, &mean.ATS, &mean.SD, &mean.GF);
		for(j=0;j<m;j++) {
			dev.MHT_U = t[j].MHT_U - mean.MHT_U;
			dev.MLT_U = t[j].MLT_U - mean.MLT_U;
			dev.MHT_P = t[j].MHT_P - mean.MHT_P;
			dev.MLT_P = t[j].MLT_P - mean.MLT_P;
			dev.ATS = t[j].ATS - mean.ATS;
			dev.SD = t[j].SD - mean.SD;
			dev.GF = t[j].GF - mean.GF;
			fprintf(list, "%lf %lf %lf %lf %lf %lf %lf %s\n", scale(dev.MHT_U), scale(dev.MLT_U), scale(dev.MHT_P), scale(dev.MLT_P), scale(dev.ATS), scale(dev.SD), scale(dev.GF), current_usr);
		}
	}
	fclose(list);
	LOCK = 0;	// Unlock
}

double scale(double d)		// Scale input value to a double fraction value within [-50 to 50]
{
	double ret = d;
	if(ret<300)
	{while(ret>20)
		ret = ret/(double)20;
		//ret=ret/(double)50;
         }
 	else
        {
	while(ret <-50 || ret>50)
		ret = ret/(double)50;
          }

	return ret;
//return d;
}

void clean_tmp_folder(void)
{
	remove("tmp/merged_log_file");
	remove("tmp/list_of_deviations");
	remove("tmp/final_dev");
	remove("tmp/read");
	remove("tmp/write");
	remove("tmp/feedfile");
	remove("tmp/templogin");
	remove("tmp/KNN_RETURN");
}

void create_deviations_final_file(char *name)
{	// 'name' passed is the username entered in login, so for these usernames in the 'list_of_deviations' file, y = 1 (i.e. correct target)
	FILE *last_list = fopen("tmp/list_of_deviations", "r");
	FILE *final = fopen("tmp/final_dev", "w");

	struct template_input dev;
	char usr[120];
	int i, n = linecount(last_list);
	for(i=0;i<n;i++)
	{
		fscanf(last_list, "%lf %lf %lf %lf %lf %lf %lf %s", &dev.MHT_U, &dev.MLT_U, &dev.MHT_P, &dev.MLT_P, &dev.ATS, &dev.SD, &dev.GF, usr);
		if(strcmp(usr, name) == 0)
			fprintf(final, "%lf %lf %lf %lf %lf %lf %lf %d\n", dev.MHT_U, dev.MLT_U, dev.MHT_P, dev.MLT_P, dev.ATS, dev.SD, dev.GF, VALID);
		else
			fprintf(final, "%lf %lf %lf %lf %lf %lf %lf %d\n", dev.MHT_U, dev.MLT_U, dev.MHT_P, dev.MLT_P, dev.ATS, dev.SD, dev.GF, INVALID);			
	}
	
	fclose(final);
}

double* feed_into_network(FILE *F)
{printf("feeding_into_network...\n");
	// Write to file: "tmp/write"
	// Read back from: "tmp/read"
	// When run, network.m should read 2 lines from "tmp/write", feed them to the NN and write the output to "tmp/read"

	FILE *write = fopen("tmp/write", "w");
	struct template_input temp;
	fscanf(F, "%lf %lf %lf %lf %lf %lf %lf", &temp.MHT_U, &temp.MLT_U, &temp.MHT_P, &temp.MLT_P, &temp.ATS, &temp.SD, &temp.GF);
printf("Print_values read: %lf %lf %lf %lf %lf %lf %lf\n", temp.MHT_U, temp.MLT_U, temp.MHT_P, temp.MLT_P, temp.ATS, temp.SD, temp.GF);
	fprintf(write, "%lf %lf %lf %lf %lf %lf %lf\n", temp.MHT_U, temp.MLT_U, temp.MHT_P, temp.MLT_P, temp.ATS, temp.SD, temp.GF);
//	fscanf(F, "%lf %lf %lf %lf %lf %lf %lf", &temp.MHT_U, &temp.MLT_U, &temp.MHT_P, &temp.MLT_P, &temp.ATS, &temp.SD, &temp.GF);
//	fprintf(write, "%lf %lf %lf %lf %lf %lf %lf\n", temp.MHT_U, temp.MLT_U, temp.MHT_P, temp.MLT_P, temp.ATS, temp.SD, temp.GF);
	fclose(write);

	system("octave network.m");
	wait(NULL);
	FILE *read = fopen("tmp/read", "r");
	double *out = calloc(2, sizeof(double));
	fscanf(read, "%lf %lf", &out[0], &out[1]);

//	remove("tmp/write");		
//	remove("tmp/read");
printf("network fed...\n");
	return out;
}

int* get_KNNVAL(void)
{
	int *array = calloc(3, sizeof(int));
	int usercount, val;
	FILE *merged;
	merged = fopen("tmp/merged_log_file", "r");

	if(merged == NULL)
	{
		create_merged_log_file();
		while(LOCK);
		merged = fopen("tmp/merged_log_file", "r");
		fscanf(merged, "%d", &usercount);
		val = 5*usercount;
		array[0] = val;
		val = val + 2*usercount;
		array[1] = val;
		val = val + 2*usercount;
		array[2] = val;
	}
	else
	{
		fscanf(merged, "%d", &usercount);
		val = 5*usercount;
		array[0] = val;
		val = val + 2*usercount;
		array[1] = val;
		val = val + 2*usercount;
		array[2] = val;
	}

	return array;
}

struct registered_users* get_registered_usernames(void)
{
	struct registered_users *reg = malloc(sizeof(struct registered_users));
	FILE *merged = fopen("tmp/merged_log_file", "r");
	if(merged == NULL)	{	create_merged_log_file();	merged = fopen("tmp/merged_log_file", "r");	}
	int M, j;
	double boss;
	fscanf(merged, "%d", &(reg->N));
	reg->name = calloc(reg->N, sizeof(char *));
	char dummy[100], trash[500];
	int i;
	for(i=0;i<reg->N;i++)	{
		reg->name[i] = malloc(100*sizeof(char));
		fscanf(merged, "%s %s %s %s", dummy, reg->name[i], dummy, dummy);
		fscanf(merged, "%d", &M);
		for(j=0;j<M;j++)
			fscanf(merged, "%lf %lf %lf %lf %lf %lf %lf", &boss, &boss, &boss, &boss, &boss, &boss, &boss);
		fscanf(merged, "%lf %lf %lf %lf %lf %lf %lf", &boss, &boss, &boss, &boss, &boss, &boss, &boss);
	}
	rewind(merged);
	fclose(merged);
	return reg;
}

struct top_users return_top_users(FILE *knnfile)
{	
}

void create_new_rule_devfile(void)
{
	LOCK = 1;
	FILE *devlist = fopen("tmp/list_of_deviations", "r");
	FILE *topusers = fopen("tmp/top_users", "r");
	FILE *new_rule_devfile = fopen("tmp/new_rule_devfile", "w");
	int i, lines = linecount(devlist);
	rewind(devlist);
//	struct devfile_input input[lines];
///////////	This part below is with the deviations
/*	for(i = 0; i<lines; i++)
		fscanf(devlist, "%lf %lf %lf %lf %lf %lf %lf %s", &input[i].dev1, &input[i].dev2, &input[i].dev3, &input[i].dev4, &input[i].dev5, &input[i].dev6, &input[i].dev7, input[i].name);
	for(i=0; i<lines; i++)
		if(strcmp(input[i].name, login_attempt_usr) == 0)
			fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 0\n", input[i].dev1, input[i].dev2, input[i].dev3, input[i].dev4, input[i].dev5, input[i].dev6, input[i].dev7);
*/
//////////	This part is with the original feature set
	char filename[120], dummy[100];
printf("login_attmpt_usr = %s\n", login_attempt_usr);
	sprintf(filename, "log/%s.log", login_attempt_usr);
	FILE *userlog = fopen(filename, "r");
	int m = linecount(userlog);
        struct devfile_input *input;
	input = calloc(m-1, sizeof(struct devfile_input));
	rewind(userlog);
	fscanf(userlog, "%s %s", dummy, dummy);
	for(i=0;i<m-1;i++)
	{	fscanf(userlog, "%lf %lf %lf %lf %lf %lf %lf", &input[i].dev1, &input[i].dev2, &input[i].dev3, &input[i].dev4, &input[i].dev5, &input[i].dev6, &input[i].dev7);
		fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 0\n", input[i].dev1, input[i].dev2, input[i].dev3, input[i].dev4, input[i].dev5, input[i].dev6, input[i].dev7);
	}

	int check = 0;
	char name[120];
        char du[23];
	int l = linecount(topusers);
	for(i=0; i<l;i++)
	{
		fscanf(topusers, "%s %s",du,name);
		//printf("%s",login_attempt_usr);
		printf("%s",name);
                 printf("hihihihihihihihih%d",strcmp(name, login_attempt_usr));
		if(strcmp(name, login_attempt_usr) == 0)	
                {check = 1;
                 }
	}
	rewind(topusers);
	
	if(check)
	{
		char name1[120], name2[120];
		rewind(topusers);
		for(i=0; i< l; i++) {
			fscanf(topusers, "%s %s", name1, name1);
			if(strcmp(name1, login_attempt_usr) != 0)	break;	}
		rewind(topusers);
		for(i=0; i<l; i++) {
			fscanf(topusers, "%s %s", name2, name2);
			if(strcmp(name2, login_attempt_usr) != 0 && strcmp(name2, name1) != 0)	break;	}

printf("1 name1 = %s :: name2 = %s\n", name1, name2);
//////////// 	This part below is with deviations
/*		for(i=0; i<lines; i++)
			if(strcmp(name1, input[i].name) == 0)
				fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 1\n", input[i].dev1, input[i].dev2, input[i].dev3, input[i].dev4, input[i].dev5, input[i].dev6, input[i].dev7);
		for(i=0; i<lines; i++)
			if(strcmp(name2, input[i].name) == 0)
				fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 1\n", input[i].dev1, input[i].dev2, input[i].dev3, input[i].dev4, input[i].dev5, input[i].dev6, input[i].dev7);
*/
//////////	This part below is with original feature set
		char logfile1[120], logfile2[120], dummy1[100];
		sprintf(logfile1, "log/%s.log", name1);
		sprintf(logfile2, "log/%s.log", name2);
		FILE *file1 = fopen(logfile1, "r");
		FILE *file2 = fopen(logfile2, "r");
		int m1 = linecount(file1);
		int m2 = linecount(file2);
		rewind(file1);
		rewind(file2);
		struct devfile_input in1[m1-1], in2[m2-1];
		fscanf(file1, "%s %s", dummy1, dummy1);
		fscanf(file2, "%s %s", dummy1, dummy1);
		for(i=0;i<m1-1;i++)
		{	fscanf(file1, "%lf %lf %lf %lf %lf %lf %lf", &in1[i].dev1, &in1[i].dev2, &in1[i].dev3, &in1[i].dev4, &in1[i].dev5, &in1[i].dev6, &in1[i].dev7);
			fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 1\n", in1[i].dev1, in1[i].dev2, in1[i].dev3, in1[i].dev4, in1[i].dev5, in1[i].dev6, in1[i].dev7);
		}
		for(i=0;i<m2-1;i++)
		{	fscanf(file2, "%lf %lf %lf %lf %lf %lf %lf", &in2[i].dev1, &in2[i].dev2, &in2[i].dev3, &in2[i].dev4, &in2[i].dev5, &in2[i].dev6, &in2[i].dev7);
			fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 1\n", in2[i].dev1, in2[i].dev2, in2[i].dev3, in2[i].dev4, in2[i].dev5, in2[i].dev6, in2[i].dev7);
		}
	}
	else
	{
		char name1[120], name2[120];
		fscanf(topusers, "%s %s", name1, name1);
		fscanf(topusers, "%s %s", name2, name2);
printf("2 name1 = %s :: name2 = %s\n", name1, name2);
//////////	This part below is with deviations
/*		for(i=0; i<lines; i++)
			if(strcmp(name1, input[i].name) == 0)
				fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 1\n", input[i].dev1, input[i].dev2, input[i].dev3, input[i].dev4, input[i].dev5, input[i].dev6, input[i].dev7);		
		for(i=0; i<lines; i++)
			if(strcmp(name2, input[i].name) == 0)
				fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 1\n", input[i].dev1, input[i].dev2, input[i].dev3, input[i].dev4, input[i].dev5, input[i].dev6, input[i].dev7);
*/
//////////	This part below is with original feature set
		char logfile1[120], logfile2[120], dummy1[100];
		sprintf(logfile1, "log/%s.log", name1);
		sprintf(logfile2, "log/%s.log", name2);
		FILE *file1 = fopen(logfile1, "r");
		FILE *file2 = fopen(logfile2, "r");
		int m1 = linecount(file1);
		int m2 = linecount(file2);
                //m1-=3;
		//m2-=3;
		rewind(file1);
		rewind(file2);
		struct devfile_input in1[m1-1], in2[m2-1];
		fscanf(file1, "%s %s", dummy1, dummy1);
		fscanf(file2, "%s %s", dummy1, dummy1);
		for(i=0;i<m1-1;i++)
		{	fscanf(file1, "%lf %lf %lf %lf %lf %lf %lf", &in1[i].dev1, &in1[i].dev2, &in1[i].dev3, &in1[i].dev4, &in1[i].dev5, &in1[i].dev6, &in1[i].dev7);
			fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 1\n", in1[i].dev1, in1[i].dev2, in1[i].dev3, in1[i].dev4, in1[i].dev5, in1[i].dev6, in1[i].dev7);
		}
		for(i=0;i<m2-1;i++)
		{	fscanf(file2, "%lf %lf %lf %lf %lf %lf %lf", &in2[i].dev1, &in2[i].dev2, &in2[i].dev3, &in2[i].dev4, &in2[i].dev5, &in2[i].dev6, &in2[i].dev7);
			fprintf(new_rule_devfile, "%lf %lf %lf %lf %lf %lf %lf 1\n", in2[i].dev1, in2[i].dev2, in2[i].dev3, in2[i].dev4, in2[i].dev5, in2[i].dev6, in2[i].dev7);
		}
	}
	fclose(new_rule_devfile);
	LOCK = 0;
}
