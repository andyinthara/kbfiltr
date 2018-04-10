// config.h
// rude::Config interface version 2.x
// $Revision: 1.10 $
// $Date: 2003/10/04 19:42:30 $
// Author: Matt Flood
// Copyright 2003 RudeServer.com
// All rights reserved


#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

namespace rude{

namespace config{
class ConfigImpl;
}

//=
// Config is the public interface for the configuration component.
//=
class Config{

	// Bridge component
	//
	rude::config::ConfigImpl *d_implementation;

public:


	//=
	// Default constructor
	// Use to obtain an instance of the rude::Config component
	// 
	// <b>Example:</b>
	// <code>
	// rude::Config myconfig;
	// 
	// //OR
	// 
	// rude::Config *myconfig = new rude::Config();
	// </code>
	//= 
	Config();

	//=
	// Returns the version of the component.
	// 
	// The version is specified by <b>X.Y</B> where: 
	// <B>X</B> is the version of the interface (Config), and
	// <B>Y</B> is the version of the implementation (ConfigImpl).
	// 
	// <b>Example:</b>
	// 
	// <code>const char *version = Config::version();</code>
	//=
	static const char *version();

	//=
	// Set's the default filepath to be used by the load() and save() methods.
	// The default filepath is initially set to "./default.ini"
	//=
	void setConfigFile(const char *filepath);
	
	//=
	// Sets whether or not deleted data will be preserved as comments.
	// If set to true, deleted data will be converted into comments when 
	// the save() method is called.  If set to false, deleted data
	// will not be compeltely discarded. Default is false (no preserve).
	//=
	void preserveDeletedData(bool shouldPreserve);

	//=
	// Set's the comment character ( initially set to '#' )
	// If the comment character is set to NULL (0), 
	// then comments will not be written when save() is called
	//=
	void setCommentCharacter(char commentchar);

	//=
	// Set's the delimiter for new config objects ( initially set to '=' )
	// If the delimiter is set to NULL, then it will be assumed that 
	// the key and value are separated by whitespace.
	//=
	void setDelimiter(char kayvaluedelimiter);

	//=
	// Saves the configuration to the default config file.
	// Initially, the default config file is "./default.ini".
	// This filename can be changed by the setConfigFile() method.
	//=
	bool save();

	//=
	// Saves the configuration object to the specified file
	// The default config file path is not altered...
	// Use setConfigFile() to permanently set the default config file for load() and save()
	//=
	bool save(const char *filepath);

	//=
	// Removes the contents of the configuration file - completely wipes it out.
	// Does not preserve anything.  Afterwards, the current section is set
	// to the empty section (""), which is the unnamed section
	// at the beginning of the configuration file.
	//=
	void clear();
	
	//=
	// Loads the default file into the configuration object.
	// By default, the current file is 'default.conf', 
	// but can be overridden by calling the setConfigFile() method.
	// This method does not clear the config object before loading.
	// If you load two or more configuration files that have the same sections,
	// they are effectively merged.  In this case, data members that have the same
	// name will get the value of the last file loaded. This comes in handy if
	// you load a global config file first, and you want to override
	// some custom fields by subsequently loading a "preferences" file tailored to an individual.
	// Use clear() if the config object already has data that you want to completely discard before loading
	// a new file, or use another Config object instance.
	//=
	bool load();

	//=
	// Adds the file's configuration info to the current configuration object.
	// Does not delete any existing data in the config object. 
	// The default config file path is not altered...
	// Use setConfigFile() to permanently set the default config file for load() and save()
	// This method does not clear the config object before loading.
	// Use clear() if the config object already has data that you want to discard.
	//=
	bool load(const char *filename);

	//=
	// Returns the most recent error string, if there is one.
	// If no error exists, it will return the empty string ("").
	//=
	const char *getError();

	//=
	// Returns the number of sections in the entire configuration file, including the default section - ""
	// Sections within the configuration file are identifed by [Square Brackets] surrounding the name of the section.
	// Whitespace surrounding the section name is ignored. So [   This Section    ] and [This Section] are identical.
	// Section names <b>are</b> case sensitive.  
	// The default section is the empty section - the unnamed section at the beginning of the file that
	// exists before any other [named] sections.
	// This default section exists even if the first line of the file is a [named section].
	// As such, the return value will always be >= 1.
	//=
	int getNumSections() const;

	//=
	// Enumeration method to discover sections
	// Returns the section name at the given index,
	// or NULL if the index is out of range
	// If the section has no name, but is a valid index, then it will return the empty string ("")
	//=
	const char *getSectionNameAt(int index) const;
	
	//=
	// Sets the current working section, possibly creating a new section
	// The default section is "" (the untitled section at the beginning of the configuration file).  
	// If the new section cannot be found, and <b>shouldCreate</b> is <b>true</b>,
	//  then the section will be created.  
	// If the new section cannot be found, and <b>shouldCreate</b> is <b>false</b>,
	//  then the current section remains unchanged, and the method returns false.
	// Leading and trailing whitespace is not preserved when the file is written,
	//  and as such should be avoided when creating sections
	//=
	bool setSection(const char *sectionname, bool shouldCreate);

	//=
	// Sets the current working section, creating the section if it does not exist.
	// See <b><i>setSection(const char *sectionname, bool shouldCreate)</i></b> where <i>shouldCreate</i> = true.
	//=
	bool setSection(const char *sectionname);

	//=
	// Deletes the section identified by <i>sectionname</i> and all data associated with it
	// Returns false if the section does not exist or has already been deleted.
	// If the object is saved, the entire section will be commented out.
	// If, during the lifetime of this object, the section is re-created, then the section name becomes uncommented, but all
	// previous data members remain commented (as if deleted) unless/until recreated (see deleteData())
	//=
	bool deleteSection(const char *sectionname);


	//=
	// Returns the number of data elements for the current section
	// <b>Example:</b>
	// 
	// Given the following configuration:
	// 
	//<font color=red>[Good Movies]
	// The 5th Element
	// Resevoir Dogs
	// Braveheart
	// 
	// [Bad Movies]
	// 
	// Freddy Got Fingered
	// CONfidential
	// 
	// [Body Parts]
	// arm=2
	// head=1
	// legs=2
	// neck=1
	// ears=2
	// eyes=2
	//	
	// </font>
	//
	// You'll get the following results:
	// <code> 
	// config->setSection("Body Parts", true);
	// config->getNumDataMembers(); // returns 6
	//
	// config->setSection("Bad Movies", true);
	// config->getNumDataMembers(); // returns 2
	//
	// config->setSection("Good Movies", true);
	// config->getNumDataMembers(); // returns 3
	// </code>
	//=
	int getNumDataMembers() const;

	//=
	// Enumeration method to discover data members within the current section
	// Returns the name of the data member at 
	// the specified index within the current
	// section, or NULL if the index is out of range.
	//
	// <b>Example:</b>
	// 
	// Given the following configuration:
	// 
	//<font color=red>[Contact Info]
	// name= Mark Twain
	// email address = mark@twain
	// phone = 123.456.789
	// </font>
	//
	// You'll get the following results:
	// <code> 
	// config->setSection("Contact Info");
	// config->getDataNameAt(0); // returns "name"
	// config->getDataNameAt(2); // returns "phone"
	// config->getDataNameAt(3); // returns NULL (out of range)
	//
	// </code>
	//=
	const char *getDataNameAt(int index) const;

	//=
	// Returns true if a data member exists with the given name within the current section
	// 
	// <b>Example:</b>
	// 
	// Given the following configuration:
	//
	//<font color=red>
	// [Strong Body Parts]
	// arm=2
	// head=1
	// legs=2
	// neck=1
	//
	// [Weak Body Parts]
	//	eyes=2
	// belly=1
	// nose=1
	// </font>
	//
	// You'll get the following results:
	// <code> 
	// config->setSection("Strong Body Parts", true);
	// 
	// if(config->exists("arm"))
	// {
	// 	// yes, arm exists...
	// }
	// 
	// if(config->exists("belly"))
	// {
	// 	// returns false - belly isn't in that section.
	// }
	// 
	// config->setSection("Weak Body Parts", true);
	// 
	// if(config->exists("belly"))
	// {
	//	// yes! belly is in this section....
 	// }
	// </code>
	//=
	bool exists(const char *name) const;

	//=
	// Returns the boolean value of the data identified by <i>name</i> within the current section
	// Returns true if the current value is "on" or 1,
	// or if the value starts with the letter 't' or 'y'
	// ( all of the above are case insensitive )
	// returns false otherwise
	//
	// As such, it will interpret the following as true:
	// Yes, yes, On, on, True, true, 1, T, t, Y, y
	//
	// Likewise, it will interpret the following as false:
	// No, no, Off, off, False, false, 0, F, f, N, n
	//
	// 
	// <b>Example:</b>
	// 
	// Given the following configuration:
	//
	//<font color=red>
	// [Things that are and aren't]
	// Bed Bugs= TRUE
	// Aliens = 1
	// Freud= ON
	// Water= yes
	// ducks = false
	// carpenter = off
	// floppy = 0
	// sand = Nope
	// </font>
	// You'll get the following results:
	// <code> 
	// config->setSection("Things that are and aren't");
	// 
	// config->getBoolValue("Aliens"); // returns true
	// config->getBoolValue("carpenter"); // returns false
	// // etc...
	// </code>
	//=
	bool getBoolValue(const char *name) const;

	//=
	// Returns the integer value of the data identifed by <i>name</i> within the current section
	// Returns 0 if the data does not exist or if an integer value cannot be determined.
	//=
	int getIntValue(const char *name) const;

	//=
	// Returns the double value, or 0 if a double value cannot be determined
	//=
	double getDoubleValue(const char *name) const;

	//=
	// Returns the string-character value.
	// Unless quoted, leading and trailing whitespace is stripped.
	//=
	const char * getStringValue(const char *name) const;

	//=
	// Sets the named value within the current section to "true" or "false"
	// If the value is already set for the data item, the value is changed.
	// If the value is not already set, it is created
	// 
	// <b>Example:</b>
	// 
	// To create the following configuration entries:
	// 
	// <font color=red>[On Sale]
	//	Hard Drives = true
	// Cases = false
	// Keyboards = false
	// </font>
	//
	// Use this code:
	// <code> 
	// config->setSection("On Sale", true);
	// config->setBoolValue("Hard Drives", true);
	// config->setBoolValue("Cases", false);
	// config->setBoolValue("Keyboards", false);
	// config->save();
	// </code>
	//
	// @param name the identifying name of the data member
	// @param value The integer value of the data member
	//
	//=
	void setBoolValue(const char *name, bool value);

	//=
	// Sets the named value within the current section to the specified integer
	// If the value is already set for the data item, the value is changed.
	// If the value is not already set, it is created
	// 
	// <b>Example:</b>
	// 
	// To create the following configuration entries:
	// 
	// <font color=red>[Inventory]
	//	Hard Drives = 35
	// Cases = 24
	// Keyboards = 103
	// </font> 
	//
	// Use this code:
	// <code> 
	// config->setSection("Inventory", true);
	// config->setIntValue("Hard Drives", 35);
	// config->setIntValue("Cases", 24);
	// config->setIntValue("Keyboards", 103);
	// config->save();
	// </code>
	//
	// @param name the identifying name of the data member
	// @param value The integer value of the data member
	//
	//=
	void setIntValue(const char *name, int value);

	//=
	// Sets the named value within the current section to the specified double
	// If the value is already set for the data item, the value is changed.
	// If the value is not already set, it is created
	// 
	// <b>Example:</b>
	// 
	// To create the following configuration entries:
	// 
	// <font color=red>[Prices]
	//	Hard Drives = 59.95
	// Cases = 44.00
	// Keyboards = 12.25
	// </font> 
	//
	// Use this code:
	// <code> 
	// config->setSection("Prices", true);
	// config->setDoubleValue("Hard Drives", 59.95);
	// config->setDoubleValue("Cases", 44.00);
	// config->setDoubleValue("Keyboards", 12.25);
	// config->save();
	// </code>
	//
	// @param name the identifying name of the data member
	// @param value The double value of the data member
	// @author Matt Flood
	//
	//=
	void setDoubleValue(const char *name, double value);

	//=
	// Sets the named value within the current section to the specified string
	// If the value is already set for the data item, the value is changed.
	// If the value is not already set, it is created.
	// If leading and/or trailing whitespace exists within the value, it will be preserved in
	// the config file by surrounding the value with quotes.
	// Likewise, if the value starts with a quote ("), then the value
	// will necessarily be surrounded by quotes to preserve it.
	//
	// Note:  Whitespace within the identifying fieldname (the key for the pair) is not preserved.
	// 
	// <b>Example:</b>
	// 
	// To create the following configuration entry:
	// 
	// <font color=red>[travel]<br>Travel Agent = Mary Johnson</font>
	// 
	// Use this code:
	// <code> 
	// config->setSection("travel", true);
	// config->setStringValue("Travel Agent", "Mary Johnson");
	// config->save();
	// </code>
	//
	// @param name the identifying name of the data member
	// @param value The string value of the data member
	// @author Matt Flood
	//
	//=
	void setStringValue(const char *name, const char *value);

	//=
	// Deletes the data member identified by <i>name</i> (if it exists) in the current section.
	// If the configuration object is saved, 
	// then the data member will be commented out.
	// If a new value is later assigned to this 
	// value (in the lifetime of the instance),
	// then it becomes undeleted (uncommented).
	//=
	bool deleteData(const char *name);

	~Config();

};

} // end namespace rude

#endif

