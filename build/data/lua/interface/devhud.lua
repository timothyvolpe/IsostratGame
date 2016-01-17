if CLIENT then
	devhud = Interface.create( "Screen" )
	devhud:SetPosition( 0, 0 )
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
	devhud_wireframe:SetText( "#DEBUG_WIREFRAME# #DEBUG_WIREFRAME_MODE0#" ); --\n#DEBUG_TRIANGLECOUNT# 0\n#DEBUG_WIREFRAME# #DEBUG_WIREFRAME_MODE0#
	devhud_wireframe:Activate()
end