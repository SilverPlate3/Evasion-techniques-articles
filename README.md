# Evasion techniques 2 - PEB edition 
## Cynet Security 

[![N|Solid](data:image/png;base64,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)](https://www.cynet.com/)
Written by: Ariel Silver

Article can be found at - 


# Intro
The article explains and shows the following
 - What is the Process Environment Block
 - Different ways adversaries can exploit it, in order to evade user-mode tools and detections.
 - Live attack scenarios that show when and how malwares should use it.
 - WinAPI's and system structures (documented and undocumented) that must be used.
 - Most important, how Cynet detects this bevavior.
 

# Code 
- The full code and solutions for these techniqes can be found in the code folder.
- The code should be compiled in x64 Relsease mode.
- In order to understand the full code you MUST read the article, as it's very detailed and nuch more in-depth then the code comments. 



Previous article - https://www.cynet.com/attack-techniques-hands-on/defense-evasion-techniques/