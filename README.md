Albuquerque
---
Albuquerque is the repository containing the sapling of a soon to be personal custom game and rendering engine and editor following the principles described
in the blog post [Write Games, Not Engines](https://geometrian.com/programming/tutorials/write-games-not-engines/).

Currently, it is being actively used to develop 'Plane Game' which is contained under 'Projects'. Eventually, as more and more games
are developed in the same codebase and shared code become interfaces and abstractions, it will slowly grow to become a game engine.

The name 'Albuquerque' was chosen as a placeholder codename inspired by this [page](https://wiki.osdev.org/Beginner_Mistakes) on the osdev wiki. Specifically, 
it was inspired by the internal codenames of Windows Operating Systems that were named after American cities. I chose Albuquerque as it is a niche and 
unknown city in New Mexico that I have a personal connection with.

Build Instructions
---
Coming Soon! Right now it only builds on Windows. If you know Cmake, just run the CMake files and cross your fingers that it works after
you precompile the three libraries.
There are three libraries that were precompiled during my prototyping. I will get around to using git submodules or Fetch_Content for these libraries in the near future.

Plane Game
---
Pre-Alpha Release Download Link for Plane Game[https://github.com/ClementineAccount/Albuquerque/releases/tag/v0.1.0-indev]

![image](https://user-images.githubusercontent.com/26779639/233835096-0425f3ee-9390-4a34-a07a-f1c95e81af42.png)

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

Third Party Placeholder Asset Credits (Plane Game)
---

Grass Texture: https://www.poliigon.com/texture/ground-forest-003/1949 

Plane SFX: https://www.soundjay.com/propeller-plane-sound-effect.html 

Background Music: https://soundcloud.com/personahofficial/personah-real-love-free-download

Skybox Textures: [Thin Matrix's Tutorial](https://www.youtube.com/watch?v=_Ix5oN8eC1E)

Pause Menu Music
```
Above The Clouds
You are free to use  Above The Clouds  music track (even for commercial purposes on social media / monetized videos), but you must include the following in your video description (copy & paste):
Above The Clouds by | e s c p | https://escp-music.bandcamp.com
Music promoted by https://www.free-stock-music.com
Creative Commons / Attribution 3.0 Unported License (CC BY 3.0)
https://creativecommons.org/licenses/by/3.0/deed.en_US
```


Library List
---

- GLFW
- GLAD
- Dear Imgui
- ImGuizmo (Not used yet)
- SoLoud
- cgltf (Not used)
- tinygltf
- FreeType (Not used yet)
- stb_image
- Tracy
- spdlog
- JPH (Not used yet)
