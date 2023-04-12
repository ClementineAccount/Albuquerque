Currently being developed with a driving game I am working on:
![image](https://user-images.githubusercontent.com/26779639/230844833-36683e8c-7033-4522-aa2b-71e276649bae.png)

This project is a self-learning exercise in understanding and implementing the complexities of how to render seamless open world environments with presistent states (level editing features) in an optimized way. It is also an exercise in learning more complex vehicle physics simulation considerations. 

In other words, the main learning outcomes of the project are:
- Open World memory management 
- Player driven level editing tools (Similar to Trackmania)
- Vehicle Physics Simulation for Gameplay (Arcadey vs Realistic Balance)
- Terrarin Generation and Dynamic Skyboxes

Therefore, these other requirements of the project are not given as high priority and uses third-party libraries as the scaffolding in current stage of development:
- Collision Detection, Shapes and Physics Responses (Uses [JoltPhysics](https://github.com/jrouwe/JoltPhysics) however we do not use the Vehicle Controller)
- Windowing, Context and Graphics API Abstraction (We use GLFW, GLAD and [Fwog](https://github.com/JuanDiegoMontoya/Fwog))
- Audio (Uses [SoLoud](https://solhsa.com/soloud/))
- Efficient .gltf file parsing and loading (Uses [tinygltf](https://github.com/syoyo/tinygltf)

Another third party library included is Lua as a potential scope for level editing may include more complex scriptable track elements or programmable AI racers but am undecided if its fully necessary yet.
