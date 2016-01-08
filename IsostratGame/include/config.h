#pragma once

#include <boost\property_tree\ptree.hpp>
#include <vector>

enum
{
	KEYBIND_WALK_FORWARD = 0,
	KEYBIND_WALK_BACKWARD,
	KEYBIND_STRAFE_LEFT,
	KEYBIND_STRAFE_RIGHT,
	KEYBIND_RUN,
	KEYBIND_WALK,
	KEYBIND_TOGGLE_DEBUGDRAW,
	KEYBIND_SET_FRUSTRUM,
	KEYBIND_COUNT
};

class CConfigLoader
{
private:
	boost::property_tree::ptree m_xmlTree;
	std::vector<unsigned short> m_keybinds;

	int m_windowX, m_windowY;
	int m_resolutionX, m_resolutionY;

	bool m_isFullScreen;
	bool m_isBorderless;

	float m_fieldOfView;
public:
	CConfigLoader();
	~CConfigLoader();

	bool saveConfig();

	bool initializeAndLoad();
	void destroy();

	int getWindowX();
	void setWindowX( int X );
	int getWindowY();
	void setWindowY( int Y );

	int getResolutionX();
	void setResolutionX( int X );
	int getResolutionY();
	void setResolutionY( int Y );

	bool isFullscreen();
	void setFullscreen( bool fullscreen );
	bool isBorderless();
	void setBorderless( bool borderless );

	float getFieldOfView();
	void setFieldOfView( float fov );

	unsigned short getKeybind( unsigned short keybind );
	void setKeybind( unsigned short keybind, unsigned short key );
};