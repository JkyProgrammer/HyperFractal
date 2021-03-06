# HyperFractal

HyperFractal is a standalone application dedicated to generating mathematical fractals. This project also serves as my Computer Science A Level NEA.
It is intended to be functional and stable, and runs on both MacOS and Windows 10 (although it may compile under other systems).

HyperFractal was designed as a teaching aid as well as a useful tool.

GUI library uses [raylib](https://www.raylib.com/index.html), along with [raygui](https://github.com/raysan5/raygui).

## Features
* Both graphical and console interfaces
* Arbitrary image size
* Allows for navigation through fractals
* Support for storing and loading render states in/from a database
* Support for custom fractal equations interpreted and evaluated at runtime, as well as a number of hard-coded presets
* Multithreaded distributed rendering, although limited to the CPU
* Full console control over render parameters

## Help & Instructions

Feeling lost? Below is an explanation of how to use the GUI mode of the application.

### Rendering

When you start the application or make changes to parameters such as zoom, equation, centering, etc, you will be shown a low-resolution preview render in the viewport on the left side of the window.

By clicking on the 'Render Image' button in the control panel, the application will then generate a screen-resolution image in the viewport, during which time parameter controls are locked. Rendering progress is shown below the 'Render Image' button.

### Parameters

Parameters determine the image which is output. These include the following:
* Zoom - allows you to look closer at particular areas by stretching the mathematical space on both axes - can be altered using the 'Zoom In', 'Reset Zoom', 'Zoom Out' buttons, or the '+' and '-' buttons on the keyboard
* Offset - allows you to move the viewport around the fractal by offsetting the mathematical space on one or more axes - can be altered using the 'up', 'down' 'left', 'right' buttons, or the arrow keys
* Equation - the iterative mathematical expression which is used to generate values for each pixel - clicking in the equation input box and typing allows you to alter this: **see dedicated help section**
* Iteration limit - limits the number of times the equation is iterated before the program assumes it does not tend to infinity - can be altered using the '<' and '>' buttons, or using the left and right square bracket keys (accelerate by holding Shift)
* Palette - can be selected from presets using the 'Colour Palettes' button

### Saving and Loading Render States

The application allows you to load or save render states. A render state is a configuration profile containing all the rendering parameters used, including zoom, offsets, iteration limit, palette, etc.

In order to save a render state, click the 'Save Render State' button, click the input box in the middle of the screen, enter a descriptive name, and press 'Save'.

To load a render state, click the 'Load Render State' button, select an available saved profile from the list, and click 'Load'. This will overwrite current settings, so if you have a nice configuration currently showing, make sure to save it first before loading another. The loading dialog also gives you the option to delete saved profiles if you want, but be careful because this is permanent.

### Equations

Fractals are generated by iteratively applying a formula to a complex number and counting the number of times it can be iterated before tending toward infinity.

This count is then used to compute a colour for a given pixel. 

Formulas (also referred to as expressions or equations) are in the format `Z = f(Z, C)`, where `Z` is referring to the iterated value, and `C` is constant. On the first iteration, `Z` and `C` are both initialised to be equal to a complex number, with components `a+bi`. `a` and `b` are the horizonal and vertical coordinates (respectively) of the pixel being computed, meaning each pixel has different initial conditions for computation.

The equation field in the application can handle brackets (`(...)`), indices (`^`), division (`/`), multiplication (`*`), addition (`+`) and subtraction (`-`), and processes them in that order. It supports the use of `z` and `c` as basic variables, as well as `x` and `y`, which behave as the real and imaginary part of `z`, and `a` and `b`, which represent the real and imaginary parts of `c`. It also supports the use of numerical (decimal) constants which can be in terms of `i`.

There is no limit to equation length or complexity, but more complex equations are likely to be more computationally expensive and increase render time significantly.

The application also contains a number of equation presets, which can be fun to explore and are good starting points if you want to come up with your own equation.
Presets run much faster than custom entered equations due to them being hard-coded.

When writing your own equations, it is advised to replace `z^2` with `z*z`, as the latter evaluates much faster in a computational environment. The same can be done for any integer power of a function.
In general, _you are advised to simplify your expression to a reasonable extent before entering it into the application, in order to reduce render time_.

