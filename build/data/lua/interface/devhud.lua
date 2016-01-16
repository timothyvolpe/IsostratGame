if CLIENT
	devhud = Interface.create( "Screen" )
	devhud:SetPosition( 0, 0 )
	devhud:SetSize( 1, 1 )
	devhud:SetBackground( 0, 0, 0, 0 )
	devhud:SetVisible( true )
	
	devhud_fps = Interface.create( "Label", devhud )
	devhud_fps:SetPosition( 0.1, 0.1 )
	devhud_fps:SetSize( 0.25, 0.09 )
	devhud_fps:SetVisible( true )
	devhud_fps:SetText( "#DEBUG_FRAMECOUNTER# 0\n#DEBUG_TRIANGLECOUNT# 0\n#DEBUG_WIREFRAME# #DEBUG_WIREFRAME_MODE0#" );
	
	function devhud_fps:update()
	end
end