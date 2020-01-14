# Rakshata

[Rakshata](https://www.rakshata.com/) is a decentralized offline manga/comic/BD reader for OS X.  
Rakshata is designed to manage your library, with content coming from either a source compatible with Rakshata's open standard or from your local stuffs that you can easily import.  
You can download a recent build [there](https://www.rakshata.com/beta).


## Spirit

Rakshata was designed as a tool to enable everyone to share, and eventually sell, their content free of the censorship of centralized actor.  
Rakshata was made open source for two reasons :

- You are never better served than by youself, if you have ideas to improve Rakshata, you don't have to explain them to me;

- Rakshata is the result of a lot of work, including reverse-engineering some public but insufficiently documented API. This RE was caused by the lack of example, and this project will hopefully save an other poor soul from having to perform this time consumming task.

The most interesting part of the codebase is probably the various subclasses for basic UI elements (RakText, RakList...) that enable easy customization of properties AppKit can be fairly rigid about.

## Formats

The various formats documentation will be made available shortly.

## Building

Rakshata requires a non-trivial amount of dependencies, which are included in the repository but packed in an archive.  
In order to build Rakshata, you need to decompress Frameworks/Frameworks.zip and in OS X/Libraries/include.zip.  
Please note that the content of Frameworks.zip need to be directly in Frameworks/ (and not in Frameworks/Frameworks/).

## Contributing

You can chat with the dev team on IRC on freenode, #Rakshata.

### Coding style

This project use the [Allman identation style](https://en.wikipedia.org/wiki/Indent_style#Allman_style), [Camel case](https://en.wikipedia.org/wiki/CamelCase) starting with a lower case.


### Code of Conduct

Please note that this project is released with a [Contributor Code of Conduct](http://contributor-covenant.org/version/1/4/).  
By participating in this project you agree to abide by its terms.
If you want to report a violation, please contact us at [this address](mailto:abuse@rakshata.com).

### Security

Rakshata was designed with security in mind, and a particular attention was given to protect Rakshata from potentially dangerous input and was regularly submitted to static analysis.  
However, in the case you find something we missed that could potentially harm our users, please send us an encrypted email at [this address](mailto:taiki@rakshata.com).
The GPG key for this address is `AD4D 9753 4993 54C3 1B19  5CCA 8597 FA21 D9F4 E395`.  
Unless you ask otherwise, you'll be credited for the discovery.

## Licence

Rakshata is licenced under the [MPL](https://mozilla.org/MPL/2.0/).  
However, an additionnal constraint is added as you're not granted the right to distribute, on a binary form :
- Your own compiled version Rakshata;
- A software heavily based, with minimal modifications, on Rakshata

on an app store Rakshata is already made available by the maintainer of the projet.
