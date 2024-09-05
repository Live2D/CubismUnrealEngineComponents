[English](NOTICE.md) / [日本語](NOTICE.ja.md)

---

# Notices

## [Restrictions] Feature Restrictions in the Alpha Version (2024-09-05)


### Level

The alpha version does not support the use of OpenWorld levels. Please use the Basic level during your usage.

### Display the model 

#### Model Textures

We have identified an issue where models with a single 1024x1024 texture do not render correctly.
To avoid this issue, please adjust the texture atlas in Cubism Editor to anything other than a single 1024x1024 (e.g., a single 2048x2048, or two 1024x1024 textures), then re-export and re-import the model.

### Viewport Display

Live2D models may not display correctly immediately after being spawned in a level.
In such cases, please enable the real-time update feature in the viewport to refresh the screen.

### Double Buffering

Double buffering for performance improvement on mobile devices is not implemented in the alpha version.

### Assets Reimport

The assets reimport feature for embedded files is not implemented in the alpha version. 
If you need to reimport, please delete the imported assets and then reimport them.

### Components

We have identified an issue where property values changed in each component are not reflected when you play after making changes. 
To avoid this issue, please re-attach the relevant component.

### Using Unreal Editor's Debugging

We have identified an issue where the values of non-existing parameters used in CubismPose cannot be correctly retrieved when using the debug. 
There is no problem during the execution of the project.

### Export

The alpha version does not support exporting on platforms other than Windows.
Support for additional platforms will be implemented sequentially.
Please note that releasing content using the alpha version is not recommended, as significant changes to the specifications may occur.

---

©Live2D