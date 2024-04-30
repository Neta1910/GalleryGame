#include "DatabaseAccess.h"
#include "sqlite3.h"
#include "Constants.h"


// ---- Callback Functions ----
int callbackAlbum(void* data, int argc, char** argv, char** azColName)
{
	auto& albumList = *static_cast<std::list<Album>*>(data);
	Album newAlbum(0, "");
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == USER_ID)
		{
			newAlbum.setOwner(atoi(argv[i]));
		}
		else if (std::string(azColName[i]) == NAME)
		{
			newAlbum.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == CREATION_DATE)
		{
			newAlbum.setCreationDate(argv[i]);
		}
	}

	albumList.push_back(newAlbum);
	return 0;
}

int callbackUser(void* data, int argc, char** argv, char** azColName)
{
	auto& userList = *static_cast<std::list<User>*>(data);
	User newUser(0, "");

	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == ID)
		{
			newUser.setId(atoi(argv[i]));
		}
		else if (std::string(azColName[i]) == NAME)
		{
			newUser.setName(argv[i]);
		}
	}

	userList.push_back(newUser);
	return 0;
}

int callbackInt(void* data, int argc, char** argv, char** azColName)
{
	if (argc == 1 && argv[0] != nullptr)
	{
		*static_cast<int*>(data) = atoi(argv[0]);
		return 0;
	}
	return 1;
}

int callbackFloat(void* data, int argc, char** argv, char** azColName)
{
	if (argv[0] == nullptr)
	{		
		return 0;
	}
	if (argc == 1)
	{
		*static_cast<float*>(data) = std::stof(argv[0]);
		return 0;
	}
	return 1;
}

int callbackPicture(void* data, int argc, char** argv, char** azColName)
{
	auto& pictureList = *static_cast<std::list<Picture>*>(data);
	Picture newPic(0, "");

	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == ID)
		{
			newPic.setId(atoi(argv[i]));
		}
		else if (std::string(azColName[i]) == NAME)
		{
			newPic.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == LOCATION)
		{
			newPic.setPath(argv[i]);
		}
		else if (std::string(azColName[i]) == CREATION_DATE)
		{
			newPic.setCreationDate(argv[i]);
		}
	}

	pictureList.push_back(newPic);
	return 0;
}


int callbackTags(void* data, int argc, char** argv, char** azColName)
{
	auto& tagsList = *static_cast<std::list<int>*>(data);

	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == USER_ID)
		{
			tagsList.push_back(atoi(argv[i]));
		}
	}	
	return 0;
}






bool DatabaseAccess::open()
{
	int fileStat = _access(DB_FILE_NAME, 0);
	std::string name_as_string = DB_FILE_NAME;
	int res = sqlite3_open(name_as_string.c_str(), &db);
	if (res != SQLITE_OK) // Check if DB opened 
	{
		db = nullptr;
		std::cerr << "Failed to open Database" << std::endl;
		return false;
	}
	if (fileStat != -1) // Create tables if database doesn't exist
	{
		// USERS TABLE
		if (!runQuery("CREATE TABLE IF NOT EXISTS USERS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL);")) 
		{
			return false;
		}
		// ALBUMS TABLE
		if (!runQuery("CREATE TABLE IF NOT EXISTS ALBUMS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, CREATION_DATE TEXT NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY (USER_ID) REFERENCES USERS(ID));")) 
		{
			return false;
		}
		// PICTURES TABLE
		if (!runQuery("CREATE TABLE IF NOT EXISTS PICTURES (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, LOCATION TEXT NOT NULL, CREATION_DATE TEXT NOT NULL, ALBUM_ID INTEGER NOT NULL, FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS(ID));")) 
		{
			return false;
		}
		// TAGS TABLE
		if (!runQuery("CREATE TABLE IF NOT EXISTS TAGS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES(ID), FOREIGN KEY(USER_ID) REFERENCES USERS(ID));")) 
		{
			return false;
		}
	}
	return true; // DB opened successfully 
}

void DatabaseAccess::close()
{
	sqlite3_close(db);
	db = nullptr;	
}

void DatabaseAccess::clear()
{
	_albums.clear();
	_users.clear();

	close(); // Reset 'db' pointer
	remove(DB_FILE_NAME);
	open(); // Recreate 'db'
}


bool DatabaseAccess::runQuery(const std::string sqlStatement)
{
	char* errorMsg = nullptr;
	bool sqlOk = sqlite3_exec(db, sqlStatement.c_str(), nullptr, nullptr, &errorMsg) == SQLITE_OK; 	// Run query on database
	if (errorMsg != nullptr) // Print error message (if exists)
	{
		std::cerr << errorMsg << std::endl;
	}
	return sqlOk;
}

bool DatabaseAccess::runCallbackQuery(const std::string sqlStatement, CALLBACK_TYPES type, void* data)
{
	char* errorMsg = nullptr; 
	bool sqlOk = false;
	switch (type)
	{
	case AlbumCallback:
		data = &_albums;	
		sqlOk = sqlite3_exec(db, sqlStatement.c_str(), callbackAlbum, data, &errorMsg) == SQLITE_OK;
		break;
	case UserCallback:
		sqlOk = sqlite3_exec(db, sqlStatement.c_str(), callbackUser, data, &errorMsg) == SQLITE_OK;
		break;
	case CountCallback:
		sqlOk = sqlite3_exec(db, sqlStatement.c_str(), callbackInt, data, &errorMsg) == SQLITE_OK;
		break;
	case FloatCallback:
		sqlOk = sqlite3_exec(db, sqlStatement.c_str(), callbackFloat, data, &errorMsg) == SQLITE_OK;
		break;
	case PictureCallback:
		sqlOk = sqlite3_exec(db, sqlStatement.c_str(), callbackPicture, data, &errorMsg) == SQLITE_OK;
		break;
	case TagsCallback:
		sqlOk = sqlite3_exec(db, sqlStatement.c_str(), callbackTags, data, &errorMsg) == SQLITE_OK;
		break;
	}

	if (errorMsg != nullptr)
	{
		std::cerr << errorMsg << std::endl;
	}

	return sqlOk;
}


const std::list<Album> DatabaseAccess::getAlbums()
{
	_albums.clear(); 
	std::string sqlStatement = "SELECT USER_ID, NAME, CREATION_DATE FROM ALBUMS;";

	runCallbackQuery(sqlStatement, AlbumCallback);
	return _albums;

}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	_albums.clear();
	std::string sqlStatement = "SELECT USER_ID, NAME, CREATION_DATE FROM ALBUMS WHERE USER_ID = '" + std::to_string(user.getId()) + "';";

	runCallbackQuery(sqlStatement, AlbumCallback);
	return _albums;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	std::string sqlStatement = "INSERT INTO ALBUMS (NAME, CREATION_DATE, USER_ID) VALUES ('" + album.getName() + "', '" + album.getCreationDate() + "', '" + std::to_string(album.getOwnerId()) + "');";
	runQuery(sqlStatement);
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	std::string sqlStatement = "DELETE FROM ALBUMS WHERE NAME = '" + albumName + "' AND USER_ID = '" + std::to_string(userId) + "'; DELETE FROM SQLITE_SEQUENCE WHERE NAME = 'ALBUMS';";
	runQuery(sqlStatement);
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	_albums.clear();
	std::string sqlStatement = "SELECT NAME FROM ALBUMS WHERE USER_ID= '" + std::to_string(userId) + "' AND NAME = '" + albumName + "';";
	runCallbackQuery(sqlStatement, AlbumCallback);
	return !_albums.empty();
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	_albums.clear();
	std::string sqlStatement = "SELECT USER_ID, NAME, CREATION_DATE FROM ALBUMS WHERE NAME = '" + albumName + "';";
	runCallbackQuery(sqlStatement, AlbumCallback);

	if (_albums.size() == 0)  // Return an empty album if there was an error 
	{
		return Album(0, "");
	}
	else // Add pictures to the active album
	{
		int id = 0;
		std::list<int> taggedUsersId;
		std::list<Picture> picturesList;
		picturesList.clear();
		taggedUsersId.clear();

		runCallbackQuery("SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "';", CountCallback, &id);
		runCallbackQuery("SELECT * FROM PICTURES WHERE ALBUM_ID = " + std::to_string(id) + ";", PictureCallback, &picturesList);

		for (auto& pic : picturesList) // Add pictures
		{
			_albums.back().addPicture(Picture(pic.getId(), pic.getName(), pic.getPath(), pic.getCreationDate())); // Add picture
			
			runCallbackQuery("SELECT USER_ID FROM TAGS WHERE PICTURE_ID = " + std::to_string(pic.getId()) + ";", TagsCallback, &taggedUsersId);
			// Add tags to picture
			for (auto& userId : taggedUsersId)
			{
				_albums.back().tagUserInPicture(userId, pic.getName()); // Add tag
			}
		}
		return _albums.back();
	}
}

void DatabaseAccess::closeAlbum(Album& pAlbum)		
{
}

void DatabaseAccess::printAlbums()
{
	_albums.clear();
	std::string sqlStatement = "SELECT USER_ID, NAME, CREATION_DATE FROM ALBUMS;";
	runCallbackQuery(sqlStatement, AlbumCallback);
	std::list<Album>::iterator it;
	std::cout << "\t<<< Albums >>>" << std::endl;
	for (it = _albums.begin(); it != _albums.end(); ++it)
	{
		std::cout << "Album name: " << it->getName() << std::endl;
		std::cout << "Owner Id: " << it->getOwnerId() << std::endl;
		std::cout << "Creation date: " << it->getCreationDate() << std::endl;
		std::cout << "------------------------------------" << std::endl;
	}
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	std::string sqlStatement = "INSERT INTO PICTURES (NAME, LOCATION, CREATION_DATE, ALBUM_ID) VALUES ('" + picture.getName() + "',  '" + picture.getPath() + "', '" + picture.getCreationDate() + "' , (SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "' )); ";
	runQuery(sqlStatement);
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{	
	runQuery("DELETE FROM PICTURES WHERE NAME = '" + pictureName + "' and ALBUM_ID = (SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "' ); DELETE FROM SQLITE_SEQUENCE WHERE NAME = 'PICTURES';");	
	runQuery("DELETE FROM TAGS WHERE PICTURE_ID=(SELECT ID FROM PICTURES WHERE ALBUM_ID=(SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "') AND NAME='" + pictureName + "'); DELETE FROM SQLITE_SEQUENCE WHERE NAME = 'TAGS';");
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string sqlStatement = "INSERT INTO TAGS (PICTURE_ID, USER_ID) VALUES ((SELECT PICTURES.ID FROM PICTURES INNER JOIN ALBUMS ON PICTURES.ALBUM_ID = ALBUMS.ID WHERE PICTURES.NAME = \"" + pictureName + "\" AND ALBUMS.NAME = \"" + albumName + "\"), " + std::to_string(userId) + ");";
	runQuery(sqlStatement);
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string sqlStatement = "DELETE FROM TAGS WHERE PICTURE_ID=(SELECT ID FROM PICTURES WHERE ALBUM_ID=(SELECT ID FROM ALBUMS WHERE NAME= '" + albumName + "') AND NAME = '" + pictureName + "' ) AND USER_ID = '" + std::to_string(userId) + "'; DELETE FROM SQLITE_SEQUENCE WHERE NAME = 'TAGS';";
	runQuery(sqlStatement);
}

void DatabaseAccess::printUsers()
{
	_users.clear();
	runCallbackQuery("SELECT * FROM USERS", UserCallback);
	std::list<User>::iterator it;
	std::cout << "\t<<< Users >>>" << std::endl;
	for (it = _users.begin(); it != _users.end(); ++it)
	{
		std::cout << "Username: " << it->getName() << std::endl;
		std::cout << "Id: " << it->getId() << std::endl;
		std::cout << "------------------------------------" << std::endl;
	}
}

User DatabaseAccess::getUser(int userId)
{
	_users.clear();
	runCallbackQuery("SELECT * FROM USERS WHERE ID = '" + std::to_string(userId) + "';", UserCallback);
	if (_users.size() == 0) 
	{
		return User(0, ""); // Return an empty user if the given ID doesn't match any users
	}
	else
	{
		return _users.back(); // Return the last added user
	}
}

void DatabaseAccess::createUser(User& user)
{
	std::string sqlStatement = "INSERT INTO USERS (ID, NAME) VALUES ('" + std::to_string(user.getId()) + "', '" + user.getName() + "' );";
	runQuery(sqlStatement);
}

void DatabaseAccess::deleteUser(const User& user)
{
	// Remove user from USERS table 	
	runQuery("DELETE FROM USERS WHERE ID= '" + std::to_string(user.getId()) + "'; DELETE FROM SQLITE_SEQUENCE WHERE NAME = 'USERS';");
	// Remove tag of user
	runQuery("DELETE FROM TAGS WHERE USER_ID= '" + std::to_string(user.getId()) + "'; DELETE FROM SQLITE_SEQUENCE WHERE NAME = 'TAGS'");
	// Remove pictures from owned albums
	runQuery("DELETE FROM PICTURES WHERE ALBUM_ID = (SELECT ID FROM ALBUMS WHERE USER_ID = '" + std::to_string(user.getId()) + "'); DELETE FROM SQLITE_SEQUENCE WHERE NAME = 'PICTURES';");
	// Remove albums
	runQuery("DELETE FROM ALBUMS WHERE USER_ID= '" + std::to_string(user.getId()) + "'; DELETE FROM SQLITE_SEQUENCE WHERE NAME = 'ALBUMS';");
}

bool DatabaseAccess::doesUserExists(int userId)
{
	_users.clear();
	runCallbackQuery("SELECT NAME FROM USERS WHERE ID = '" + std::to_string(userId) + "';", UserCallback);
	return _users.empty(); // If the user is found and added to the list, he exists (return true)
						   // If not, the list will remain empty (return false)
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	_albums.clear();
	int count = 0;
	// Create a list of albums owned by given user
	runCallbackQuery("SELECT ID FROM ALBUMS WHERE USER_ID = '" + std::to_string(user.getId()) + "';", AlbumCallback); 
	std::list<Album>::iterator it;
	for (it = _albums.begin(); it != _albums.end(); ++it) // Count the number of albums in the list
	{
		count++;
	}
	return count; 
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	int count = 0;	
	runCallbackQuery("SELECT COUNT(DISTINCT ALBUMS.ID) FROM ALBUMS INNER JOIN PICTURES ON ALBUMS.ID=PICTURES.ALBUM_ID	INNER JOIN TAGS	ON PICTURES.ID=TAGS.PICTURE_ID WHERE TAGS.USER_ID = '" + std::to_string(user.getId()) + "';", CountCallback, &count);
	return count;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	int count = 0;	
	runCallbackQuery("SELECT COUNT(ID) FROM TAGS WHERE USER_ID= '" + std::to_string(user.getId()) + "';", CountCallback, &count);
	return count;
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	float average = 0;	
	runCallbackQuery("SELECT 1.0*COUNT(PICTURE_ID)/COUNT(DISTINCT ALBUMS.ID) FROM ALBUMS INNER JOIN PICTURES ON ALBUMS.ID=PICTURES.ALBUM_ID INNER JOIN TAGS ON PICTURES.ID=TAGS.PICTURE_ID WHERE TAGS.USER_ID = '" + std::to_string(user.getId()) + "';", FloatCallback, &average);
	return average;
}

User DatabaseAccess::getTopTaggedUser()
{
	_users.clear(); 
	runCallbackQuery("SELECT ID, NAME FROM USERS WHERE ID = (SELECT USER_ID FROM TAGS GROUP BY USER_ID ORDER BY COUNT(ID) DESC LIMIT 1); ", UserCallback);
	if (_users.size() == 0)
	{
		return User(0, ""); // Return an empty user 
	}
	else
	{
		return _users.back(); // Return the top tagged user
	}
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	std::list<Picture> picturesList;
	runCallbackQuery("SELECT ID, NAME, LOCATION, CREATION_DATE FROM PICTURES WHERE ID=(SELECT PICTURE_ID FROM TAGS GROUP BY PICTURE_ID ORDER BY COUNT(ID) DESC LIMIT 1);", PictureCallback, &picturesList);
	if (picturesList.size() == 0) 
	{
		return Picture(0, ""); // Return an empty picture 
	}
	else
	{
		return picturesList.back(); // Return the top tagged picture
	}
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	std::list<Picture> picturesList; 
	runCallbackQuery("SELECT PICTURES.ID, NAME, LOCATION, CREATION_DATE FROM PICTURES INNER JOIN TAGS ON TAGS.PICTURE_ID = PICTURES.ID WHERE TAGS.USER_ID = '" + std::to_string(user.getId()) + "';", PictureCallback, &picturesList);
	return picturesList; // Return list of pictures tagged by given user
}

