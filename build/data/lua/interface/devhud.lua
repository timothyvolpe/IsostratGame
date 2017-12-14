if CLIENT then
	devhud = Interface.create( "Screen" )
	devhud:SetPosition( 0.01, 0.91 )
	devhud:SetSize( 1, 1 )
	devhud:SetBackground( 0, 0, 0, 0 )
	devhud:SetVisible( true )
	devhud:Activate()
	
	devhud_fps = Interface.create( "Label", devhud )
	devhud_fps:SetPosition( 0, 0 )
	devhud_fps:SetSize( 0.4, 0.4 )
	devhud_fps:SetVisible( true )
	devhud_fps:SetText( "#DEBUG_FRAMECOUNTER# 0" ); --\n#DEBUG_TRIANGLECOUNT# 0\n#DEBUG_WIREFRAME# #DEBUG_WIREFRAME_MODE0#
	devhud_fps:RegisterEvent( "Update", function( self )
		devhud_fps:SetText( "#DEBUG_FRAMECOUNTER#"..math.floor(1 / Game:GetFrameTime()) )
	end )
	devhud_fps:Activate()
	
	devhud_wireframe = Interface.create( "Label", devhud )
	devhud_wireframe:SetPosition( 0, 0.03 )
	devhud_wireframe:SetSize( 0.4, 0.4 )
	devhud_wireframe:SetVisible( true )
	devhud_wireframe:RegisterEvent( "Update", function( self )
		if( Game:GetWireframeMode() == 0 ) then
			devhud_wireframe:SetText( "#DEBUG_WIREFRAME# #DEBUG_WIREFRAME_MODE0#" ) --\n#DEBUG_TRIANGLECOUNT# 0\n#DEBUG_WIREFRAME# #DEBUG_WIREFRAME_MODE0#
		elseif( Game:GetWireframeMode() == 1 ) then
			devhud_wireframe:SetText( "#DEBUG_WIREFRAME# #DEBUG_WIREFRAME_MODE1#" )
		elseif( Game:GetWireframeMode() == 2 ) then
			devhud_wireframe:SetText( "#DEBUG_WIREFRAME# #DEBUG_WIREFRAME_MODE2#" )
		else
			devhud_wireframe:SetText( "#DEBUG_WIREFRAME#" )
		end
	end )
	devhud_wireframe:Activate()
	
	devhud_position = Interface.create( "Label", devhud )
	devhud_position:SetPosition( 0, 0.06 )
	devhud_position:SetSize( 0.4, 0.4 )
	devhud_position:SetVisible( true )
	devhud_position:SetText( "#DEBUG_POSITION# (0.000, 0.000, 0.000)" );
	devhud_position:RegisterEvent( "Update", function( self )
		local cameraPos = Game:GetCameraPosition()
		cameraPos.x = tonumber( string.format( "%.3f", cameraPos.x ) )
		cameraPos.y = tonumber( string.format( "%.3f", cameraPos.y ) )
		cameraPos.z = tonumber( string.format( "%.3f", cameraPos.z ) )
		devhud_position:SetText( "#DEBUG_POSITION#".." ("..cameraPos.x..", "..cameraPos.y..", "..cameraPos.z..")" )
	end )
	devhud_position:Activate()
end