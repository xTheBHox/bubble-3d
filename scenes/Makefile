
BLENDER="Z:\Program Files\Blender Foundation\Blender\blender.exe"

all : \
	..\dist\bubble-parts.pnct \
	..\dist\bubble-level-1.scene


..\dist\bubble-parts.pnct: bubble.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- bubble.blend:Parts $@

..\dist\bubble-level-1.scene: bubble.blend export-scene.py
	$(BLENDER) --background --python export-scene.py -- bubble.blend:Level.001 $@

#../dist/city.scene : city.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- city.blend:Scene '$@'
#../dist/brunch.pnct : brunch.blend export-meshes.py
#	$(BLENDER) --background --python export-meshes.py -- brunch.blend '$@'
#../dist/brunch.scene : brunch.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- brunch.blend '$@'

#../dist/garden.pnct : garden.blend export-meshes.py
#	$(BLENDER) --background --python export-meshes.py -- garden.blend '$@'
#../dist/garden.scene : garden.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- garden.blend '$@'

#hexapod.pnct : hexapod.blend export-meshes.py
#	$(BLENDER) --background --python export-meshes.py -- hexapod.blend:Main '$@'
#hexapod.scene : hexapod.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- hexapod.blend:Main '$@'
