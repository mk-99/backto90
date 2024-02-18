# Back to 90
This project is to extract data stored in old Fidonet echo areas (squish database format), convert the data to actual database and make it available for future generations.

## Project structure

- sqetl/ - Squish database message extractor/transformer/loader
- fakemsg/ - generates fake messages, for testing

## Executables

- sqetl/makesqbase - make echo area in squish format from plain text
- sqetl/extractsqbase - extract messages from squish echo area to plain text or json format
- fakemsg/fakemsg.py - generate fake messages in plain text or json format