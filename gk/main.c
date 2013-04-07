// GKRS - A simple Keystroke recognition module in GNOME
//
// AuThOrS:	Abhinav Choudhury
//		Sambit Chatterjee
//
// This is the main body of the program.
////////////////////////////////////////////////////////

#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include "functions.h"
//#include "recog_module.h"
//#include <time.h>

//time_t tm;

int main( int argc, char **argv )
{
	remove("log/kdata_usr.log");
	remove("log/kdata_pass.log");
	clean_tmp_folder();
//	f = fopen("log.txt", "w");
	strcpy(last_usr, "");
	strcpy(last_pass, "");
	LEVEL = get_conf_settings();
printf("LEVEL = %d\n", LEVEL);
    /* Init GTK+ */
    gtk_init( &argc, &argv );

    /* Create new GtkBuilder object */
    builder_main = gtk_builder_new();
//	builder_login = gtk_builder_new();
	builder_register = gtk_builder_new();

    /* Load UI from file. If error occurs, report it and quit application.
     * Replace "tut.glade" with your saved project. */
    if( ! gtk_builder_add_from_file( builder_main, "keystroke.glade", &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return( 1 );
    }

/*    if( ! gtk_builder_add_from_file( builder_register, "register.glade", &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return( 1 );
    }
*/
    /* Get main window pointer from UI */
    window_main = GTK_WIDGET( gtk_builder_get_object( builder_main, "Main_Window" ) );
//	window_login = GTK_WIDGET(gtk_builder_get_object(builder_login, "Login_Window"));
//	window_register = GTK_WIDGET(gtk_builder_get_object(builder_register, "Register_Window"));

    /* Connect signals */
    gtk_builder_connect_signals( builder_main, NULL );
//	gtk_builder_connect_signals(builder_login, NULL);
//	gtk_builder_connect_signals(builder_register, NULL);

    /* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder_main ) );
//	g_object_unref( G_OBJECT( builder_login ) );
    /* Show window. All other widgets are automatically shown by GtkBuilder */
    gtk_widget_show( window_main );

    /* Start main loop */
    gtk_main();

    return( 0 );
}

int get_conf_settings(void)
{
	FILE *temp;	// Both if and else parts may need to be changed based on whether the format of settings.conf is changed.

	if((temp = fopen("settings.conf", "r")) == NULL)	// "If settings.conf" does not exist
	{	temp = fopen("settings.conf", "w");	fprintf(temp, "\\\\ Note: \"level\" can only be 1, 2, 3, 4 or 5.\nlevel = 4");	fclose(temp);	return 4;	}
	else
	{
		char buf[10];
		while(strcmp(buf, "level") != 0)	fscanf(temp, "%s", buf);
		int c = getc(temp) - (int)'0';
		while(c != 1 && c!= 2 && c!=3 && c!= 4 && c!= 5)	// BUG: Might crash here if level <=0 or level >= 6.
			c = getc(temp) - (int)'0';
		fclose(temp);
		return c;
	}
}
