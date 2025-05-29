[English](README.md) / [日本語](README.ja.md)

---

# Cubism Unreal Engine Components

Welcome to the open components of the Cubism SDK for Unreal Engine.

It is used in conjunction with the Live2D Cubism Core.

Go [here](https://www.live2d.com/en/sdk/download/unrealengine/) if you're looking for the download page of the SDK package.

## License

Please read the [license](LICENSE.md) before use.

## Notices

Please read the [notices](NOTICE.md) before use.

## Compatibility with Cubism 5 new features and previous Cubism SDK versions

This SDK is compatible with Cubism 5.  
For SDK compatibility with new features in Cubism 5 Editor, please refer to [here](https://docs.live2d.com/en/cubism-sdk-manual/cubism-5-new-functions/).  
For compatibility with previous versions of Cubism SDK, please refer to [here](https://docs.live2d.com/en/cubism-sdk-manual/compatibility-with-cubism-5/).


## Structure

### Components

The components are grouped by their role, and this grouping is reflected in both the folder structure and namespaces.

#### Framework

Components and classes in this group provide additional functionality like lip-syncing, as well as integration of "foreign" Cubism files with Unreal Engine.  All the framework code is located in `./Source/Live2DCubismFramework`.

#### Rendering

Components and classes in this group provide the functionality for rendering Cubism models using Unreal Engine functionality and are located in `./Source/Live2DCubismFramework/Public/Rendering` and `./Source/Live2DCubismFramework/Private/Rendering`.

### Editor Extensions

Unreal Engine Editor extensions are located in `./Source/Live2DCubismFrameworkEditor`.

### Importer

Turning Cubism files into uasset file are located in `./Source/Live2DCubismFrameworkImporter`.

### Resources

Resources like shaders and other assets are located in `./Content/Materials`.

## Development environment

| Unreal Engine | Version |
| --- | --- |
| Unreal Editor 5.5 | 5.5.4 |
| Unreal Editor 5.4 | 5.4.4 |
| Unreal Editor 5.3 | 5.3.2 |

| Library / Tool | Version |
| --- | --- |
| Visual Studio 2022 | 17.14.1 |
| Windows SDK | 10.0.22621.0 |

## Tested environment

| Platform | Version |
| --- | --- |
| Windows 11 | 24H2 |

## Branches

If you're looking for the latest features and/or fixes, all development takes place in the `develop` branch.

The `master` branch is brought into sync with the `develop` branch once for every official SDK release.

## Usage

Create the Unreal project in C++, and after creating a `Plugins` folder under the Unreal project, copy the package folder into the created `Plugins` folder and restart Unreal Engine.
For more details, please refer to the [official manual](https://docs.live2d.com/en/cubism-sdk-manual/cubism-sdk-for-unreal-engine/).

## Contributing

There are many ways to contribute to the project: logging bugs, submitting pull requests on this GitHub, and reporting issues and making suggestions in Live2D CommUnreal Engine.

### Forking And Pull Requests

We very much appreciate your pull requests, whether they bring fixes, improvements, or even new features. To keep the main repository as clean as possible, create a personal fork and feature branches there as needed.

### Bugs

We are regularly checking issue-reports and feature requests at Live2D CommUnreal Engine. Before filing a bug report, please do a search in Live2D CommUnreal Engine to see if the issue-report or feature request has already been posted. If you find your issue already exists, make relevant comments and add your reaction.

### Suggestions

We're also interested in your feedback for the future of the SDK. You can submit a suggestion or feature request at Live2D CommUnreal Engine. To make this process more effective, we're asking that you include more information to help define them more clearly.

## Coding Guidelines

We adhere to the official Unreal Engine coding standard. [Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine)

## Forum

If you have any questions, please join the official Live2D forum and discuss with other users.

- [Live2D Creators Forum](https://community.live2d.com/)
- [Live2D 公式クリエイターズフォーラム (Japanese)](https://creatorsforum.live2d.com/)
