#Rakshata

[Rakshata][website] is a decentralized offline manga/comic/BD reader for OS X.  
Rakshata is designed to manage your library, with content coming from either a source compatible with Rakshata's open standard or from your local stuffs that you can easily import.
[website]:https://www.rakshata.com/

##Spirit

Rakshata was designed as a tool to enable everyone to share, and eventually sell, their content free of the censorship of centralized actor.  
Rakshata was made open source for two reasons :

- You are never better served than by youself, if you have ideas to improve Rakshata, you don't have to explain them to me;

- Rakshata is the result of a lot of work, including reverse-engineering some public but insufficiently documented API. This RE was caused by the lack of example, and this project will hopefully save an other poor soul from having to perform this time consumming task.

The most interesting part of the codebase is probably the various subclasses for basic UI elements (RakText, RakList...) that enable easy customization of properties AppKit can be fairly rigid about.

##Formats

The various formats documentation will be made available shortly.

##Building

Rakshata require a non-trivial amount of dependencies, which are included in the repository but packed in an archive.  
In order to build Rakshata, you need to decompress Frameworks/Frameworks.zip and in OS X/Libraries/include.zip.  
Please note that the content of Frameworks.zip need to be directly in Frameworks/ (and not in Frameworks/Frameworks/*.frameworks).

##Contributing

You can chat with the dev team on IRC on freenode, #Rakshata

###Coding style

This project use the [Allman identation style][allman-ident], [Camel case][camel-case] starting with a lower case.

[allman-ident]: https://en.wikipedia.org/wiki/Indent_style#Allman_style
[camel-case]: https://en.wikipedia.org/wiki/CamelCase

###Code of Conduct

Please note that this project is released with a [Contributor Code of Conduct][code-of-conduct].  
By participating in this project you agree to abide by its terms.  
If you want to report a violation, please contact us at [this address][abuse-address].
[code-of-conduct]: http://contributor-covenant.org/version/1/4/
[abuse-address]:mailto:abuse@rakshata.com

##Licence

Rakshata is licenced under the [MPL][mpl-licence].  
However, an additionnal constraint is added as you're not granted the right to distribute, on a binary form :
- Your own compiled version Rakshata;
- A software heavily based, with minimal modifications, on Rakshata

on an app store Rakshata is already made available by the maintainer of the projet.
[mpl-licence]:https://mozilla.org/MPL/2.0/
