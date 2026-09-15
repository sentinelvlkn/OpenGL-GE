# Project Documentation

This folder contains technical documentation for the game engine.

These docs explain:
- the engine's structure
- game rooms authoring
- how Blender data is converted to runtime data
- what main functions do
- how to build and debug the project

The project is organized around a set of shared state objects and focused systems.

## Documentation files

- **ENGINE_OVERVIEW.md** - high-level explanation of the engine
- **STATE_OBJECTS.md** - explanation of the main runtime state objects
- **MAIN_LOOP_FLOW.md** - startup, per-frame and shutdown order
- **CODE_STRUCTURE.md** - what each source file/system is responsible for
- **FUNCTION_REFERENCE.md** - lookup reference for important public functions
- **ROOM_AND_BLENDER_PPELINE.md** - room JSON, Blender data and room authoring pipeline
- **BUILD_AND_DEPENDENCES.md** - dependences, build steps and common issues during build

## License

Original code in this project is licensed under the ... License.

Third-party components remain under their respective licenses.

See:

- `LICENSE`
- `THIRD_PARTY_NOTICES.md`
- `third_party_licenses/`

## Acknowledgements

This engine started as a learning project.
The main learning source is [LearnOpenGL](https://learnopengl.com/) by Joey de Vries.
Thus, some early rendering, shader, model-loading and OpenGL concepts were learned, adapted from and based on the LearnOpenGL tutorials.

For third-party libraries, tools, fonts, assets, and licenses, see:
```text
THIRD_PARTY_NOTICES.md
```



