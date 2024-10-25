# Model Scriber

- **This tool is experimental and should be used with caution.**
- If you encounter any issues, please first consider whether the problem may be related to the tool itself, rather than a potential user error.
- If the issue you are encountering is related to the tool itself, please consider opening an issue. Include a step-by-step explanation of the problem and attach the corresponding files you are using.
- This tool wasn't designed for editing pre-existing game models, but rather for creating new ones. Therefore, if you attempt to replace existing models, it is essential to understand the game's file structure.

## Command-line Options

<table>
  <thead>
    <tr>
      <th>Option</th>
      <th>Description</th>
      <th>Example</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>-fbx &lt;file&gt; [required]</code></td>
      <td>FBX input file that will be used for model scriber.</td>
      <td><code>-fbx "model.fbx"</code></td>
    </tr>
    <tr>
      <td><code>-outfile &lt;file&gt; [optional]</code></td>
      <td>Output file where model scriber will be written.</td>
      <td><code>-outfile "C:\model.perm.bin"</code></td>
    </tr>
    <tr>
      <td><code>-vertexdecl &lt;name&gt; [required]</code></td>
      <td>Specify the vertex declaration type, check list below.</td>
      <td><code>-vertexdecl Skinned</code></td>
    </tr>
    <tr>
      <td><code>-bonepalette &lt;file&gt; [optional]</code></td>
      <td>Bone palette file, the file must be just chunk of the actual bone palette!</td>
      <td><code>-bonepalette "BonePalette.perm.bin"</code></td>
    </tr>
    <tr>
      <td><code>-name &lt;model_name&gt; [optional]</code></td>
      <td>Name of the model, otherwise uses filename of FBX input file.</td>
      <td><code>-name "ALEX_SKIN_BODY"</code></td>
    </tr>
    <tr>
      <td><code>-texturescriber [optional]</code></td>
      <td>Enables writing config file that can be used with TextureScriber.</td>
      <td><code>-texturescriber</code></td>
    </tr>
  </tbody>
</table>

## Vertex Declarations

<table>
  <thead>
    <tr>
      <th>Name</th>
      <th>Elements</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>UVN</td>
      <td>Position, TexCoord0, Normal</td>
    </tr>
    <tr>
      <td>UVNTC</td>
      <td>Position, TexCoord0, Normal, Tangent, Color0</td>
    </tr>
    <tr>
      <td>Skinned</td>
      <td>Position, Normal, Tangent, Blend Index, Blend Weight, TexCoord0</td>
    </tr>
  </tbody>
</table>