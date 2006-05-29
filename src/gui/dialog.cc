/** @file
 Dialog - class with various static dialogs:<br>
 openFileDialog - pick a filename for opening it<br>
 saveFileDialog - pick a filename for saving as<br>
 openFileDialogPdf, saveFileDialogPdf - specialization of above for PDF files<br>
 colorDialog - dialog for selecting color<br>
 readStringDialog - Ask user a question end expect him to enter some string as answer<br>
 @author Martin Petricek
*/
#include "dialog.h"
#include "settings.h"
#include "version.h"
#include <qdir.h> 
#include <qinputdialog.h> 
#include <qmessagebox.h> 
#include <qfiledialog.h> 
#include <qcolordialog.h>
#include <qfileinfo.h> 
#include <utils/debug.h>
#include <qstring.h>
#include "util.h"

namespace gui {

/**
 Get current directory from File Dialog
 @param fd File Dialog
 @return current directory
 */
QString getDir(QFileDialog &fd) {
 const QDir *d=fd.dir();
 QString name=d->absPath();
 delete d;
 return name;
}

/**
 Generic "Open file" dialog.
 Wait for user to select one existing file and return its name.
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param caption Caption used in dialog title
 @param settingName Key in settings used to save/load dialog window position
 @param savePath Key in settings used to identify where to save last used directory in the dialog
 @param filters Filters available to user to restrict shown filetypes in dialog
 @return selected Filename (or NULL)
*/
QString openFileDialog(QWidget* parent,const QString &caption/*=QString::null*/,const QString &settingName/*=QString::null*/,const QString &filters/*=QString::null*/,const QString &savePath/*=QString::null*/) {
 guiPrintDbg(debug::DBG_DBG,"openFileDialog invoked");
 QFileDialog fd(parent,"openfiledialog",TRUE);
 if (!filters.isNull()) {
  //Set filters if filters specified
  fd.setFilter(filters);
 }
 fd.setShowHiddenFiles(TRUE);
 if (!caption.isNull()) fd.setCaption(caption);
 if (savePath.isNull()) {
  //No save path specified -> start in current directory
  fd.setDir(".");
 } else {
  //Try to set last used saved path, if it exists
  fd.setDir(globalSettings->read("history/path/"+savePath,"."));
 }
 fd.setMode(QFileDialog::ExistingFile);

 // "Infinite" loop, to restart the dialog if necessary (invalid file selected, etc ... )
 for(;;) {
  if (!settingName.isNull()) {
   // Restore window position from settings if applicable
   globalSettings->restoreWindow(&fd,settingName);
  }
  if (fd.exec()==QDialog::Accepted) {	//Dialog accepted
   if (!settingName.isNull()) {
    // Save window position to settings if applicable
    globalSettings->saveWindow(&fd,settingName);
   }
   if (!savePath.isNull()) {
    //Save the path if desired
    if (globalSettings->readBool("history/save_filePath",true)) {
     //Note that there is only one central setting "save paths in dialog" for all dialog types
     globalSettings->write("history/path/"+savePath,getDir(fd));
    }
   }
   QString name=fd.selectedFile();
    if (QFileInfo(name).isDir()) { //directory was selected
     //TODO: test this !
     fd.setDir(name);
     continue;//restart dialog
    }
   return name;
  }
  return QString::null;
 }
}

/**
 Invoke "open file" dialog for PDF files. Specialization of openFileDialog
 \see openFileDialog
 @param parent Parent widget - will be disabled during the dialog.
 @return selected Filename (or NULL)
*/
QString openFileDialogPdf(QWidget* parent) {
 return openFileDialog(parent,QObject::tr("Open file ..."),"file_dialog",QObject::tr("PDF files (*.pdf)"),"filePath");
}

/**
 Invoke generic "save file" dialog. Wait for user to select or type a single file and return its name.
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param oldname Name of file to be saved - if specified, this name will be pre-selected.
 @param askOverwrite If true and selected file exists, user will be asked to confirm overwriting it
 @param caption Caption used in dialog title
 @param settingName Key in settings used to save/load dialog window position
 @param savePath Key in settings used to identify where to save last used directory in the dialog
 @param filters Filters available to user to restrict shown filetypes in dialog
 @return selected Filename (or NULL)
*/
QString saveFileDialog(QWidget* parent,const QString &oldname,bool askOverwrite/*=true*/,const QString &caption/*=QString::null*/,const QString &settingName/*=QString::null*/,const QString &filters/*=QString::null*/,const QString &savePath/*=QString::null*/) {
 guiPrintDbg(debug::DBG_DBG,"saveFileDialog invoked");
 QFileDialog fd(parent,"savefiledialog",TRUE);
 if (!filters.isNull()) {
  //Set filters if filters specified
  fd.setFilter(filters);
 }
 fd.setShowHiddenFiles(TRUE);
 if (!caption.isNull()) fd.setCaption(caption);
 if (savePath.isNull()) {
  //No save path specified -> start in current directory
  fd.setDir(".");
 } else {
  //Try to set last used saved path, if it exists
  fd.setDir(globalSettings->read("history/path/"+savePath,"."));
 }
 if (!oldname.isNull()) fd.setSelection(oldname);
 fd.setMode(QFileDialog::AnyFile);
 //Name that will hold the file (if some is picked)
 QString name;
 // "Infinite" loop, to restart the dialog if necessary (invalid file selected, etc ... )
 for(;;) {
  if (!settingName.isNull()) {
   // Restore window position from settings if applicable
   globalSettings->restoreWindow(&fd,settingName);
  }
  if (fd.exec()==QDialog::Accepted) {
   if (!settingName.isNull()) {
    // Save window position to settings if applicable
    globalSettings->saveWindow(&fd,settingName);
   }
   name=fd.selectedFile();
   //TODO: check if not directory
   if (askOverwrite && QFile::exists(name)) {
    //File exists : ask if it should be overwritten
    int answer=QMessageBox::question(parent,APP_NAME,QObject::tr("File \"")+name+QObject::tr("\" already exists. Overwrite?"),
                                     QObject::tr("&Yes"),QObject::tr("&No"),QObject::tr("&Cancel"),1,2);
    if (answer==0) {				 //Yes, overwrite is ok
     //Break from the cycle mean valid file was selected
     break;
    }
    if (answer==1) continue;			//No, restart dialog and ask for another file
    if (answer==2) return QString::null;	//Cancel, do not overwrite and exit
   }
   //Not asking about overwrite
   //Break from the cycle mean valid file was selected
   break;
  }
  //Dialog cancelled
  if (!settingName.isNull()) {
   // Save window position to settings if applicable
   globalSettings->saveWindow(&fd,settingName);
  }
  return QString::null;
 } //End of not-so-infinite for cycle

 if (!savePath.isNull()) {
  //Save the path if desired
  if (globalSettings->readBool("history/save_filePath",true)) {
   //Note that there is only one central setting "save paths in dialog" for all dialog types
   globalSettings->write("history/path/"+savePath,getDir(fd));
  }
 }
 return name;
}

/**
 Invoke "save file" dialog for PDF files. Specialization of saveFileDialog
 \see saveFileDialog
 @param parent Parent widget - will be disabled during the dialog.
 @param oldname Name of file to be saved - if specified, this name will be pre-selected.
 @param askOverwrite If true and selected file exists, user will be asked to confirm overwriting it
 @return selected Filename (or NULL)
*/
QString saveFileDialogPdf(QWidget* parent,const QString &oldname,bool askOverwrite/*=true*/) {
 return saveFileDialog(parent,oldname,askOverwrite,QObject::tr("Save file as ..."),"file_dialog",QObject::tr("PDF files (*.pdf)"),"filePath");
}

/**
 Invoke "read string" dialog. Show message and wait for user to type any string, 
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param message Message to show in the dialog
 @param def Default text that will be pre-typed in the dialog
 @return typed text, or NULL if dialog cancelled
 */
QString readStringDialog(QWidget* parent,const QString &message, const QString &def) {
 bool ok=FALSE;
 QString res=QInputDialog::getText(APP_NAME,message,QLineEdit::Normal,def,&ok,parent,"read_string");
 if (ok) return res;
 return QString::null;
}

/**
 Invoke dialog to select color.
 Last selected color is remembered and offered as default next time.
 The 'initial default color' is red
 @param parent Parent widget - will be disabled during the dialog.
 @return selected color, or last used color if the dialog was cancelled
*/
QColor colorDialog(QWidget* parent) {
 static QColor defaultColor=Qt::red;
 QColor ret=QColorDialog::getColor(defaultColor,parent,"std_color_dialog");
 if (ret.isValid()) defaultColor=ret;
 return defaultColor;
}

} // namespace gui
