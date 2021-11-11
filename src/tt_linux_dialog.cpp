#ifdef __linux__
#include			<gtk/gtk.h>
#include			<string>
//https://stackoverflow.com/questions/6145910/cross-platform-native-open-save-file-dialogs
//https://github.com/AndrewBelt/osdialog
struct				osdialog_filters
{
	const char *name, **patterns;
	int npatterns;
};
bool				osdialog_file(bool open, const char *default_path, const char *filename, osdialog_filters *filters, int nfilters, std::string &result)
{
	if(!gtk_init_check(NULL, NULL))
		return 0;

	GtkFileChooserAction gtkAction;
	const char* title;
	const char* acceptText;
	//if(action == OSDIALOG_OPEN)
	if(open)
	{
		title = "Open File";
		acceptText = "Open";
		gtkAction = GTK_FILE_CHOOSER_ACTION_OPEN;
	}
	//else if (action == OSDIALOG_OPEN_DIR)
	//{
	//	title = "Open Folder";
	//	acceptText = "Open Folder";
	//	gtkAction = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	//}
	else
	{
		title = "Save File";
		acceptText = "Save";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SAVE;
	}

	GtkWidget* dialog = gtk_file_chooser_dialog_new(title, NULL, gtkAction,
		"_Cancel", GTK_RESPONSE_CANCEL,
		acceptText, GTK_RESPONSE_ACCEPT, NULL);

	for(int k=0;k<nfilters;++k)
	{
		auto filter=filters+k;
		auto fileFilter=gtk_file_filter_new();
		gtk_file_filter_set_name(fileFilter, filter->name);
		for(int k2=0;k2<filter->npatterns;++k2)
			gtk_file_filter_add_pattern(fileFilter, filter->patterns[k2]);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), fileFilter);
	}

	if(!open)
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if(default_path)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), default_path);

	if(!open&&filename)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);

	bool ret=false;
	char *chosen_filename=nullptr;
	if(ret=gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT)
		chosen_filename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	gtk_widget_destroy(dialog);

	if (chosen_filename)
		result=chosen_filename;

	while(gtk_events_pending())
		gtk_main_iteration();
	return ret;
}

//https://github.com/mlabbe/nativefiledialog
#if 0
static void		AddTypeToFilterName( const char *typebuf, char *filterName, size_t bufsize )
{
    const char SEP[] = ", ";

    size_t len = strlen(filterName);
    if ( len != 0 )
    {
        strncat( filterName, SEP, bufsize - len - 1 );
        len += strlen(SEP);
    }
    
    strncat( filterName, typebuf, bufsize - len - 1 );
}
static void		dialog_add_filters(GtkWidget *dialog, const char *filter_list)
{
    GtkFileFilter *filter;
    char typebuf[NFD_MAX_STRLEN] = {0};
    const char *p_filterList = filterList;
    char *p_typebuf = typebuf;
    char filterName[NFD_MAX_STRLEN] = {0};
    
    if ( !filterList || strlen(filterList) == 0 )
        return;

    filter = gtk_file_filter_new();
    for(;;)
    {
        
        if ( NFDi_IsFilterSegmentChar(*p_filterList) )
        {
            char typebufWildcard[NFD_MAX_STRLEN];
            /* add another type to the filter */
            assert( strlen(typebuf) > 0 );
            assert( strlen(typebuf) < NFD_MAX_STRLEN-1 );
            
            snprintf( typebufWildcard, NFD_MAX_STRLEN, "*.%s", typebuf );
            AddTypeToFilterName( typebuf, filterName, NFD_MAX_STRLEN );
            
            gtk_file_filter_add_pattern( filter, typebufWildcard );
            
            p_typebuf = typebuf;
            memset( typebuf, 0, sizeof(char) * NFD_MAX_STRLEN );
        }
        
        if ( *p_filterList == ';' || *p_filterList == '\0' )
        {
            /* end of filter -- add it to the dialog */
            
            gtk_file_filter_set_name( filter, filterName );
            gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog), filter );

            filterName[0] = '\0';

            if ( *p_filterList == '\0' )
                break;

            filter = gtk_file_filter_new();            
        }

        if ( !NFDi_IsFilterSegmentChar( *p_filterList ) )
        {
            *p_typebuf = *p_filterList;
            p_typebuf++;
        }

        p_filterList++;
    }
    
    //always append a wildcard option to the end

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter, "*.*" );
    gtk_file_filter_add_pattern( filter, "*" );
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog), filter );
}
bool			dialog_openfile(const char *default_path, const char *filter_list, std::string &result)
{
	if(!gtk_init_check(nullptr, nullptr))
		return false;
	GtkWidget *dialog=nullptr;
	if(open)
		dialog=gtk_file_chooser_dialog_new("Open", nullptr, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, nullptr);
	else
		dialog=gtk_file_chooser_dialog_new("Save As", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, nullptr);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);
	
	dialog_add_filters(dialog, filter_list);
	if(default_path)
		gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(dialog), default_path);

    bool ret=gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT;
    if(ret)
        result=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    WaitForCleanup();
    gtk_widget_destroy(dialog);
    WaitForCleanup();
    
    return ret;
}
#endif
#endif
