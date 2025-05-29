# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).


## [5-r.1-beta.1] - 2025-05-29

### Added

* Added support for drawing to render targets.
* Added a sample demonstrating Cubism model rendering on the HUD.
* Added a Blueprint function to spawn a model at runtime using a CubismModel3Json asset.
* Added CubismUpdateControllerComponent for centralized update control of Cubism components.
* Added CubismUpdatableInterface to unify update behavior across Cubism components.
* Added editor toggle options to enable or disable Physics, EyeBlink, and Pose behavior while editing in the Unreal Editor.

### Fixed

* Fix initial motion and expression not playing when Index is set in CubismMotionComponent and CubismExpressionComponent.
* Fix an issue with missing header files when compiling with Unreal Automation Tool.
* Fix errors when spawning Cubism model in open world levels.
* Fix an error occurring when starting PIE.
* Fix an error triggered by Undo actions.
* Fix to stop motion if the index is set to -1.
* Fix crash when creating Blueprint from CubismModel.

### Changed

* Modified to allow hiding Cubism models in world space.
* Renamed shader class used for mask rendering.
* Updated samples to support user interaction through the UI.
* Refactor Cubism components to implement CubismUpdatableInterface and delegate updates to CubismUpdateControllerComponent.
* Adjust GetExecutionOrder values across components to ensure deterministic update order.


## [5-r.1-alpha.5] - 2025-01-30

### Added

* Add editor to controll Cubism model parameters.
* Add constructors to structs used in Json classes.

### Changed

* Hide CubismPartComponents and CubismParameterComponents from the component hierarchy.
* Change CubismLipSyncComponent to inherit from ActorComponent.
* Reorganize Tick function execution order.

### Removed

* Remove CubismUserDataComponent and move its responsibilities to CubismDrawableComponent.
* Remove unnecessary UnregisterComponent calls.

### Fixed

* Fix editor crash when vertex attributes are empty.
* Fix an issue where C2509 would occur during Windows package builds.


### Fixed

* Fixed an issue that caused delays in the drawing masks.


## [5-r.1-alpha.4] - 2024-12-05

### Added

* Add motion curve validations to `motion3.json` importer. by [@pillowtrucker](https://github.com/Live2D/CubismNativeFramework/pull/57)
* Add feature to reimport Cubism assets.
* Due to incompatibility with this update, support for Unreal Engine 5.1 will be discontinued.

### Fixed

* Fix texture misidentification as normal maps.


## [5-r.1-alpha.3] - 2024-10-31

### Changed

* Optimize mask texture rendering.
* Optimize model mesh rendering.


## [5-r.1-alpha.2] - 2024-09-26

### Added

* Add support for Unreal Engine 5.1 and 5.4.

### Changed

* Align component addition order with execution order.

### Fixed

* Fix the calculation of CubismMotionComponent.
* Fix incorrect pose transition behavior.


## [5-r.1-alpha.1] - 2024-09-05

### Added

* New released!


[5-r.1-beta.1]: https://github.com/Live2D/CubismUnrealEngineComponents/compare/5-r.1-alpha.4...5-r.1-beta.1
[5-r.1-alpha.5]: https://github.com/Live2D/CubismUnrealEngineComponents/compare/5-r.1-alpha.4...5-r.1-alpha.5
[5-r.1-alpha.4]: https://github.com/Live2D/CubismUnrealEngineComponents/compare/5-r.1-alpha.3...5-r.1-alpha.4
[5-r.1-alpha.3]: https://github.com/Live2D/CubismUnrealEngineComponents/compare/5-r.1-alpha.2...5-r.1-alpha.3
[5-r.1-alpha.2]: https://github.com/Live2D/CubismUnrealEngineComponents/compare/5-r.1-alpha.1...5-r.1-alpha.2
