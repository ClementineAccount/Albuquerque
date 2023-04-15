Gonna rescope the game from driving to flying as flying has easier collision response code when not using a physics library. Will revisit driving in the future instead.

Currently being developed with a driving game I am working on (placeholder screenshot):
![image](https://user-images.githubusercontent.com/26779639/232227588-52cd5a3d-2f6b-4640-9692-2ed51f427a6c.png)

This project is a self-learning exercise in understanding and implementing the complexities of how to render seamless open world environments with presistent states (level editing features) in an optimized way. It is also an exercise in learning more complex vehicle physics simulation considerations. 

In other words, the main learning outcomes of the project are:
- Open World memory management 
- Player driven level editing tools (Similar to Trackmania)
- Vehicle Physics Simulation for Gameplay (Arcadey vs Realistic Balance)
- Terrarin Generation and Dynamic Skyboxes

Therefore, these other requirements of the project are not given as high priority and uses third-party libraries as the scaffolding in current stage of development:
- Windowing, Context and Graphics API Abstraction (We use GLFW, GLAD and [Fwog](https://github.com/JuanDiegoMontoya/Fwog))
- Audio (Uses [SoLoud](https://solhsa.com/soloud/))
- Efficient .gltf file parsing and loading (Uses [tinygltf](https://github.com/syoyo/tinygltf))
- Dear Imgui for editor side UI. Might not implement a separate playerside UI. 

Another third party library included is Lua as a potential scope for level editing may include more complex scriptable track elements or programmable AI racers but am undecided if its fully necessary yet.


Third Party Placeholder Asset Credits: 

Grass Texture: https://www.poliigon.com/texture/ground-forest-003/1949

Plane SFX: https://www.soundjay.com/propeller-plane-sound-effect.html

Background Music: https://soundcloud.com/personahofficial/personah-real-love-free-download
