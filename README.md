This is a simple vectorial drawing app written in C++/QT
It allows the following operations:

1. Draw a Line, Rectangle, Circle, Arc or Polygons

2. Shape Selection
  Once the user clicks with the mouse inside a Shape he has the following options:
  - select drag the shape
  - resize vertically or horizontally by pulling one of the 4 hooks
    (works for Circle/Ellipse, Rectangle and Polygon)
    (the same action for the Line shape it allows to rotate the line)
  - ZoomIn ZoomOut the shape using the mouse wheel
  - remove a selected shape;
  - clone a selectes shape (Copy/Paste)
  - change the background or the foreground color of a shape
  - change the pen width, pen style, pen color and brush color

3. Group shapes
   By click and drag over shapes it is possible to group them and perform the
   following actions on them together:
  - drag the whole group
  - ZoomIn ZoomOut using the wheel
  - Clone the whole group

4. Persistency
  - save current work that load it later

Remark:
-some parts of the code were written using Google Gemini AI

![App Screenshot](./screenshots/Screenshot2.png)
![App Screenshot](./screenshots/screenshot1.png)

