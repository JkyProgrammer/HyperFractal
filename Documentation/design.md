# Design

## High Level Design

As mentioned in the analysis section, I decided to allow both command line and GUI operation of my application. As such, my design can be effectively split into two main parts:
* the fractal generator
* the GUI

The fractal generator is the core of the application which does the actual processing and creation of the rendered image. It will be accessible from both command line and GUI modes, whereby either mode can create a rendering environment with the necessarry parameters (zoom, offset, equation, etc) which can be used for rendering universally.

In CLI mode, this can be done by parsing command line arguments and entering them into the rendering environment as parameters.

In GUI mode, this can be done instead by simply reading out values from the interface which the user can manipulate (e.g. the user presses a button which zooms in and this command is processed by changing a parameter in the rendering environment).

The GUI is effectively a separate module which provides the graphical interface to the user and simply modifies and calls into the rendering environment provided by the core module and displays the results on the screen.


The rendering environment will be encapsulated in a class which can be instantiated to create an independent, contained environment for generating fractals. It will contain class variables which can be assigned or retreived via getters and setters. It will also provide functionality to render fractals and to parse textual equations (see below), hiding all the code necessarry to do this from anything which utilises the environment (e.g. the GUI module).
// TODO: make HFractal class variables private

The GUI environment will also be encapsulated in a class, which is host to the entire graphical system. It will be responsible for creating and managing the main application window, handling button presses and key events, and will keep track of the rendering environments (two will be used, one for the low resolution preview render, the other for the main, full-resolution render). The GUI class will contain various methods for handling presses for each button, navigation events like moving the viewport, as well as the main loop function which will manage drawing the interface each frame (this can also be broken down into separate methods). The only externally accessible method will be the GUI class's main function, which will perform setup such as intialising the class, creating the GUI and starting the graphics mainloop.

The `int main(int, char**)` function, which is called when the program starts, will contain code to handle either a console start or a GUI start. For a console start, it will parse the command line arguments, configure a rendering environment, run the render, save the result, and exit. For a GUI start, instead it will simply construct a GUI class and transfer control to the graphics main thread.

## Additional Detail

## Class Digram

## Flow Diagram