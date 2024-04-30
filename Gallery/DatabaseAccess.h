#pragma once
#include "IDataAccess.h"
#include "sqlite3.h"
#include "Album.h"
#include "User.h"
#include "Picture.h"
#include <list>
#include <io.h>

// --- SQL_QUERIES ---
#define GET_ALBUMS "SELECT * FROM ALBUMS"
#define GET_ALBUMS_OF_USER "SELECT * FROM ALBUMS INNER JOIN USERS ON USERS."	

#define DB_FILE_NAME "Gallery.sqlite"

enum CALLBACK_TYPES {AlbumCallback, UserCallback, CountCallback, FloatCallback, PictureCallback, TagsCallback};

class DatabaseAccess : public IDataAccess
{
public:
	DatabaseAccess() = default;
	virtual ~DatabaseAccess() = default;

	// album related
	const std::list<Album> getAlbums() override;
	const std::list<Album> getAlbumsOfUser(const User& user) override;
	void createAlbum(const Album& album) override;
	void deleteAlbum(const std::string& albumName, int userId) override;
	bool doesAlbumExists(const std::string& albumName, int userId) override;
	Album openAlbum(const std::string& albumName) override;
	void closeAlbum(Album& pAlbum) override;
	void printAlbums() override;

	// picture related
	void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override;
	void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override;
	void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;
	void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;

	// user related
	void printUsers() override;
	User getUser(int userId) override;
	void createUser(User& user) override;
	void deleteUser(const User& user) override;
	bool doesUserExists(int userId) override;


	// user statistics
	int countAlbumsOwnedOfUser(const User& user) override;
	int countAlbumsTaggedOfUser(const User& user) override;
	int countTagsOfUser(const User& user) override;
	float averageTagsPerAlbumOfUser(const User& user) override;

	// queries
	User getTopTaggedUser() override;
	Picture getTopTaggedPicture() override;
	std::list<Picture> getTaggedPicturesOfUser(const User& user) override;

	bool open() override;
	void close() override;
	void clear() override;

private:
	sqlite3* db; // Pointer to the gallerys' database 	
	bool runQuery(const std::string sqlStatement);
	bool runCallbackQuery(const std::string sqlStatement, CALLBACK_TYPES type, void* data = nullptr);

	std::list<Album> _albums;
	std::list<User> _users;
};