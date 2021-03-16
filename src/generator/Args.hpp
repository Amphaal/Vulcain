// Vulcain
// Toy project for Vulkan oriented graphics
// Copyright (C) 2021 Guillaume Vara <guillaume.vara@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <vector>
#include <string_view>
#include <iostream>
#include <filesystem>

class Args {
 public:
    Args(int argc, char *argv[]) {
        //
        auto args = _argsToStrViews(argc, argv);
        if(args.size() < 2) {
            throw std::logic_error("You must provide at least 2 arguments !");
        }

        //
        _fillDestinationDirectory(args);

        //
        _fillTBReflectedFiles(args);
    }

    std::filesystem::path destinationDirectory;
    std::vector<std::filesystem::path> toReflectSPRIRVFiles;
 
 private:
    void _fillDestinationDirectory(std::vector<std::string_view>& args) {
        destinationDirectory = std::filesystem::path(args.back());
        args.pop_back();

        if(!std::filesystem::is_directory(destinationDirectory)) {
            //
            if(std::filesystem::exists(destinationDirectory)) {
                throw std::logic_error("Last argument must be a destination directory.");
            }
            
            //
            std::filesystem::create_directories(destinationDirectory);
        }
        
        destinationDirectory = std::filesystem::absolute(destinationDirectory);
    }

    void _fillTBReflectedFiles(const std::vector<std::string_view>& args) {
        toReflectSPRIRVFiles.reserve(args.size());

        for(auto const &i : args) {
            std::filesystem::path path(i);
            path = std::filesystem::absolute(path);

            if(!std::filesystem::exists(path)) {
                throw std::logic_error("“" + path.string() + "” must be an existing file !");
            }

            if(std::filesystem::is_directory(path)) {
                throw std::logic_error("“" + path.string() + "” is expected to be a file !");
            }

            toReflectSPRIRVFiles.push_back(path);
        }
    }

    static std::vector<std::string_view> _argsToStrViews(int argc, char *argv[]) {
        std::vector<std::string_view> args;
        args.reserve(argc - 1);

        for(int i = 1; i < argc; i++) {
            auto &view = args.emplace_back(argv[i]);
        }

        return args;
    }
};
