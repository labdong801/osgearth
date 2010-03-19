/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2009 Pelican Ventures, Inc.
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osg/io_utils>

#include <osgEarth/Common>
#include <osgEarth/Map>
#include <osgEarth/CacheSeed>
#include <osgEarth/EarthFile>
#include <osgEarth/Registry>

#include <iostream>
#include <sstream>

using namespace osgEarth;

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser args(&argc,argv);

    args.getApplicationUsage()->setApplicationName(args.getApplicationName());
    args.getApplicationUsage()->setDescription(args.getApplicationName() + std::string(" is an application used to seed a cache for an osgEarth."));
    args.getApplicationUsage()->setCommandLineUsage(args.getApplicationName()+ std::string(" [options] filename"));
    args.getApplicationUsage()->addCommandLineOption("--min-level level","The minimum level to seed down to.");
    args.getApplicationUsage()->addCommandLineOption("--max-level level","The maximum level to seed down to.");
    args.getApplicationUsage()->addCommandLineOption("--bounds minx miny maxx maxy","The geospatial extents to seed.");
    args.getApplicationUsage()->addCommandLineOption("-b","Shorthand for --bounds.");
    args.getApplicationUsage()->addCommandLineOption("--cache-path","Use a different cache path than the one defined in the earth file");
    args.getApplicationUsage()->addCommandLineOption("--cache-type","Override the cache type if you override the cache path (tms or disk).");

    // if user request help write it out to cout.
    if (args.read("-h") || args.read("--help") || args.argc() <= 1)
    {
        args.getApplicationUsage()->write(std::cout);
        return 1;
    }

    
    //Read the min level
    unsigned int minLevel = 0;
    while (args.read("--min-level", minLevel));
    
    //Read the max level
    unsigned int maxLevel = 5;
    while (args.read("--max-level", maxLevel));
    
    //Read the bounds
    Bounds bounds(0, 0, 0, 0);
    while (args.read("--bounds", bounds.xMin(), bounds.yMin(), bounds.xMax(), bounds.yMax()));
    while (args.read("-b", bounds.xMin(), bounds.yMin(), bounds.xMax(), bounds.yMax()));

    //Read the cache override directory
    std::string cachePath;
    while (args.read("--cache-path", cachePath));

    //Read the cache type
    std::string cacheType;
    while (args.read("--cache-type", cacheType));

    std::string filename;
    //Find the input filename
    for(int pos=1;pos<args.argc();++pos)
    {
        if (!args.isOption(pos))
        {
            filename = args[pos];
        }
    } 

    //Make sure the user specified a file
    if ( filename.empty() )
    {
        OE_NOTICE << "Please specify a .earth file to seed." << std::endl;
        return 1;
    }

    // Register the Cache Override
    if (!cachePath.empty())
    {
        osg::ref_ptr< Cache > cache;
        if (cacheType == "disk")
        {
            OE_NOTICE << "Creating DiskCache" << std::endl;
            cache = new DiskCache( cachePath );
        }
        else
        {
            OE_NOTICE << "Creating TMSCache" << std::endl;
            cache = new TMSCache( cachePath );
        }

        OE_NOTICE <<"Override Cache Path: "<<cachePath<<std::endl;
        Registry::instance()->setCacheOverride(cache.get());
    }

    //Load the map file
    EarthFile earthFile;
    if ( earthFile.readXML( filename ) )
    {
        //Create the CacheSeed
        CacheSeed seed;
        seed.setMinLevel(minLevel);
        seed.setMaxLevel(maxLevel);
        seed.setBounds(bounds);
        seed.seed( earthFile.getMap() );
        return 0;
    }
    else
    {
        OE_NOTICE << "Could not load earth file from " << filename << std::endl;
        return 1;
    }
}
