# Odin to Fmod Adapter for Unreal Engine

This Unreal Engine sample project shows how to implement and use a class that passes the [Odin Voice Chat](https://www.4players.io/odin/)'s data to the [FMOD Audio Engine](https://www.fmod.com/docs/2.03/unreal/welcome.html).

![FMOD and Odin](/Documentation/fmod_with_odin_header.jpg)

## Pre-requisities

To be able to use the project you will need to have access to FMOD Studio and their Unreal Plugin. You can get them [here](https://www.fmod.com/download#fmodforunreal).
Also you will need to install the Odin Voice Chat Plugin for Unreal, available [here](https://github.com/4Players/odin-sdk-unreal/releases) or in the [Unreal Marketplace](https://www.unrealengine.com/marketplace/en-US/product/4players-odin-sdk).

In order to fully understand the sample and the code feel free to visit our [detailed integration guide](https://www.4players.io/odin/guides/unreal/odin-fmod-unreal/).

## Quick Start

The class is implemented in [this header](/Source/OdinTestProject/OdinFmodAdapter.h) and [this source file](/Source/OdinTestProject/OdinFmodAdapter.cpp). It can be assigned in your Player Character as a Component to use the Player Character's positional data for any attenuation. See the `Content/OdinContent/Blueprints/BP_OdinCharacter.uasset` in the sample project for reference. The `Content/OdinContent/Blueprints/C_OdinClient.uasset` blueprint showcases how to start passing incoming Voice Chat streams to FMOD using the provided adapter-class.
