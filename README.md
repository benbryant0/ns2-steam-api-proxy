# Proxy steam_api.dll for Natural Selection 2
This project is a replacement steam_api.dll for the game Natural Selection 2, exposing functionality for use by another program. At the moment all that is exposed is what is required to make the game connect to a server and to open a URL in the Steam overlay browser. This is used by my other project, [NS2 Assistant](https://github.com/benbryant0/ns2-assistant).

The connection functionality was required for another project of mine which included the ability to assist the user with joining servers. This used to be possible with the Steam browser protocol (`steam://connect/<address>`) but either Steam or NS2 made a change and it no longer works. NS2's game servers accept query requests on a different port than the actual game server's port. So `steam://connect/<address>` would query the query port, but when this was passed to the game it would attempt to connect to the query port instead of the game server port, and fail.

The dll built by this project works as a proxy for the original dll. If the original is updated by NS2, this will likely not work and need rebuilding. If the original steam_api.dll is renamed to steam_api_original.dll and this dll is named steam_api.dll and placed in NS2's root directory, it will redirect all but one function to the original dll. The function that is overridden is responsible for registering callbacks for functionality provided by Steam. The overridden function grabs the callback that the game gives to Steam to be called when Steam wants to direct NS2 to connect to a server, and passes everything onto the original dll unmodified. This callback can then be called through a named pipe that my other project connects to. This dll receives the server information and then calls the function NS2 registered with Steam which triggers a connection to the server in-game.

To open a URL in the overlay browser, two other functions are called from the original dll.

All of this is operated using a very basic packet system over a named pipe which NS2 Assistant connects to.

I had only tinkered with C++ before this and learnt as I went, so I'm likely unaware of many common conventions and better ways of doing things, and have probably made some mistakes. Please point them out!
