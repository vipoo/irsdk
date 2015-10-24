irsdk
=====

Clone of the offical iRacing SDK C++ library

====

The following is from the iRacing members forum: http://members.iracing.com/jforum/posts/list/1470675.page

Welcome to the new iRacing SDK, this api replaces the original iRacing Telemetry API with a cleaner implementation that supports multiple clients and an expanded list of functions. 

history: 
2011/04/27 - initial release 

2011/05/03 - Maintenance update 
- Fixed a conflict between Windows UAC and live telemetry, applications will need to be recompiled to continue working. 
- Fixed a bug in irsdk_broadcastMsg() that could cause the caller to hang indefinitely. 
- Telemetry now properly output from a replay 
- Removed the CarIdxThrottlePct, CarIdxBrakePct, and CarIdxClutchPct variables. 
- Fixed a bug that caused camera data to be erratic when pausing the simulator. 
- Added a small sample demonstrating how to read in an ibt file and convert it to a csv file. 

2011/05/06 - Maintenance update 
- Fixed a bug in the YAML parser that would not parse numbers greater than 9 in array entries {} 
- Initial stub of a C# port, probably not worth looking at. 

2013/01/15 - Maintenance update 
- Added new remote call 'irsdk_BroadcastReloadTextures' to trigger a reload of custom car textures. 

2013/07/22 - Maintenance update 
- Added new remote call 'irsdk_BroadcastChatComand' to help automate chat commands. 
- Added new remote call 'irsdk_BroadcastPitCommand' to allow custom manipulation of pit stop settings. 

2014/04/23 - Maintenance update
- Added new remote call 'irsdk_BroadcastTelemCommand' to allow toggling on/off of disk based telemetry 

2015/09/23 - Maintenance update
- Add new telemetry defines to irsdk_defines.h
