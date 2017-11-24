Assignment 2 - Particle Engine

Shawn Matthews
Stephen Richards
Paul Puig

Introduction:

    Our particle engine is written on top of a modified version of TTK, but the underlying simulation systems can be adapted to any rendering system. It makes use of a deferred rendering system to accumulate particles into a buffer that can later be drawn over the screen. Particles are rendered in screen-space using a geometry shader.

    The most important files are Renderer.cpp and the Particle* files.

Using the Editor:

	The editor is broken into multiple windows. The top menu bar allow you to save, load, and create new effects under the 'File' menu item. You can also open the texture manager and change the background color under the 'View' tab. 

	The texture editor allows you to rename and load new particle textures. See File Dialogs for more info about the file dialog. 

	The Effect Editor window is where the actual customization takes place. It is split into 2 main sections: Editor Controls (under a collapsable heading), and Effect Configuration. The Editor Controls lets you control playback and debug settings. Most of these are self-explainitory, but Debug Paths will show path lines and forces effecting the particles.

	The Configuration area lets you specify a name for the effect, change it's position, and manipulate layers and global behaviours. Layers are a collection of particles that share physical, visual, and emission properties. These are the core peice of the particle engine. They can be named, and can also be re-arranged by clicking the up/down arrows beside controls. Layers can be removed by pressing delete, and can be added by pressing the plus button next to the layers heading.

	To edit a layer, click Edit below the layer's name. This will open another ImGui Window, the Layer Editor. This window is broken into 4 collapsing headers: Physics, Emitter, Visuals, and Behaviours. 

	'Physics' contains physics settings for the layer like gravity, velocity, mass, and angular velocities. Velocity for emitted particles is calculated by determing a normal to a random position within the sphere specified by Velocity Radius and Velocity offset. This normal is then multiplied by a random value withing the Velocity Range to calculate a final initial velocity.

	'Emitter' contains emission settings for the layer. The position is specified relative to the parent effect's position. Duration is the duration of emission in seconds. The emitter type specifies the area that the emitter will create particles in, while Emitter Meta specifies the bounds of the shape (ignored for Point). The other fields are fairly self-explainitory. When updating the maximum number of particles for a layer, you will need to click Apply to apply your changes.

	'Visuals' contains the visual properties for the particles. The initial and final colors can be modified by expanding the drop down and selecting a color from the palette. Color interpolation can be disabled by unchecking Interpolate Color. You can also specify the blend mode and the texture to use (blending is currently buggy, see Known Issues). The blend mode is per layer. Layers above will blend into those below using their blend mode.

	'Behaviours' lets you add and modify behaviours specific to that layer. For more information see 'Behaviours' below.

Behaviours:

	 Behviours exist in both the global effect scope, and within local layer scopes. Effect-level behaviours will influence all particles in the effect, while layer-level will influence only those particles in the effect. The configuration for the behaviours is exactly the same.

	 Behaviours have 3 main properties: Name, Weight, and Method. Weight for behaviours specifies how much this behaviour should contribute to the final force vector. This is done by taking a sum of all weights and dividing the resulting vector by the sum. The method specifies how the behaviour will effect particles. Seek and flee will repel/attract particles to/from the specified point in world space, regardless of distance. Repel and Attract will repel and attract particles unless they are within a certain range. 

	 The path method will make the particles attempt to follow a path. You must specify more than one point to define a path. These points are defined in world space. You will also need to specify a path 'radius', which determines how close a particle will need to get to a path point to 'activate' it.

File Dialogs:

	Our editor uses custom files dialogs for some operations. These custom dialogues are restricted to the working directory that the application is rendering under, to help ensure that file paths can be kept relative. Note that for any save dialogues, you currently CANNOT select a sub-folder to save into, all saves will go into the root (see Known Issues)


Known Issues:

	- Cannot select sub-directory to save to for save dialogues
	- Occasional Application crash when removing items from lists in ImGui
	- Attract/Repel will sometimes behave strangely
	- Particles do not always follow paths correctly
	- Blending sometimes does not look correct
