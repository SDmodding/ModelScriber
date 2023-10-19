## Blender Script
If you use blender you can get this script: https://github.com/SDmodding/ModelScriberBlender

## Before use

You need to get those files in the folder:
- [ModelScriber.exe](https://github.com/SDmodding/ModelScriber/releases)
- [TextureScriberPC64.exe](https://github.com/SDmodding/TextureScriber/releases)

If you have everything in the folder then open command line in that folder.

## Model
`ScribeModel [Model Name] [Texture Name] [1 (If using normal map)]`
- Model Name:
    - You need to have folder with that name under `Models`
    - That folder need to contain `Model.obj`
- Texture Name:
    - Use texture name that you gonna use also with `ScribeTexture`
    - If the texture is not shared with other models just use same name as model name.

## Texture
`ScribeTexture [Texture Name] [Path (Optional)]`
- Texture Name:
    - You need to have folder with that name under `Models` or `Textures (When shared)`
    - Supported files (If you have TGA file rename extension to PNG):
        - Diffuse.png
        - Normal.png
- Path:
    - If you gonna use the model in some certain path you need to define it!
    - This will output texture under folder `Data\...`
    - The game uses the path for resource UID, when changing the path you need to scribe the texture again with new path!
