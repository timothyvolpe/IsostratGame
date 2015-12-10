#pragma once

#include <boost\property_tree\ptree.hpp>

class CConfigLoader
{
private:
	boost::property_tree::ptree m_xmlTree;

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
};