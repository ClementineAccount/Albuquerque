Gonna rescope the game from driving to flying as flying has easier collision response code when not using a physics library. Will revisit driving in the future instead.

Currently being developed with a driving game I am working on (placeholder screenshot):


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

Library List:

- GLFW
- GLAD
- Dear Imgui
- ImGuizmo
- SoLoud
- tinygltf
- FreeType
- stb_image
- Tracy
- spdlog


Third Party Placeholder Asset Credits: 
Grass Texture: https://www.poliigon.com/texture/ground-forest-003/1949
Plane SFX: https://www.soundjay.com/propeller-plane-sound-effect.html
Background Music: https://soundcloud.com/personahofficial/personah-real-love-free-download

Level Editor Music:
Above The Clouds
You are free to use  Above The Clouds  music track (even for commercial purposes on social media / monetized videos), but you must include the following in your video description (copy & paste):
Above The Clouds by | e s c p | https://escp-music.bandcamp.com
Music promoted by https://www.free-stock-music.com
Creative Commons / Attribution 3.0 Unported License (CC BY 3.0)
https://creativecommons.org/licenses/by/3.0/deed.en_US
