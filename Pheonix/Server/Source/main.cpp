#include <Core/Voxels/Blocks.hpp>
#include <Server/Main.hpp>
#include <iostream>
#include <stdio.h>

#undef main

using namespace pheonix;

//CLI arguments
//[0] save name
void main(int argc, char* argv[])
{
	voxels::BlockRegistry registry = voxels::BlockRegistry();
	registry.registerBlock("core:dirt", "Dirt");
	registry.registerBlock("core:cobble", "CobbleStone");
	registry.registerBlock("core:stone", "Stone");
	// TODO: Replace these manual calls to register blocks with a call to run passed lua files

	/* -=- Begin Sudo draft -=-
	Save::Load(argv[0]) //This will detect internally if a new map needs generated

	std::thread listener (Network::Listener); //Start listener thread to listen for connections
	
	bool run = true;
	while (run == true){
		// Listen for CLI instructions
	}

	Send signal for listener to terminate

	Confirm map has saved

	*/

	return;
};