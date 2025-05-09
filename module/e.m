E: module
{
	PATH: con "$E";


	GrOp : adt {
		fill : array of ref Level;
		svg  : string;
	
#		newSVG: fn(svg : string);
#		getSVG: fn() : string;

#		clean : fn();
	};

	
	Level : adt {
#		note: string;

		graphics : GrOp;

		new:	fn(): ref Level;

#		getPos: fn(): int;
#		insPos: fn(n: int);
#		setPos: fn(n: int): Level;
#		swpPos: fn(n: int);

#		rmPos : fn();

	};

	Shape : adt {
		id : int;

		LineWidth: fn(this: self ref Shape, w: real)                            : int;
		LineColor: fn(this: self ref Shape, r: real, g: real, b: real, a: real) : int;
		FillColor: fn(this: self ref Shape, r: real, g: real, b: real, a: real) : int;
		
		MoveTo:	   fn(this: self ref Shape, x: real, y: real)    : int;
		LineTo:	   fn(this: self ref Shape, x: real, y: real)    : int;
		CurveTo:   fn(this: self ref Shape, x1: real, y1: real,
			                                x2: real, y2: real,
			                                x3: real, y3: real)  : int;

		ArcCW:     fn(this: self ref Shape, x: real, y: real,
			                                radius: real,
			                                angle_from: real, angle_to: real) : int;
		ArcCCW:    fn(this: self ref Shape, x: real, y: real,
			                                radius: real,
			                                angle_from: real, angle_to: real) : int;


		ClosePath:	fn(this: self ref Shape)                     : int;

		setAsMask:  fn(this: self ref Shape, is_mask: int)       : int;

		setAngle:   fn(this: self ref Shape, angle: real)        : int;
		setScale:   fn(this: self ref Shape, sx: real, sy: real) : int;
		setOffset:  fn(this: self ref Shape, x: real, y: real)   : int;

		
		setDrawZ:	fn(this: self ref Shape, shape: ref Shape, n: int)  : (int, int);
		insDrawZ:	fn(this: self ref Shape, shape: ref Shape, n: int)  : (int, int);
		getDrawZ:   fn(this: self ref Shape, n: int)			 : (ref Shape, int);
		rmDrawZ:	fn(this: self ref Shape, n: int)			 : (ref Shape, int);


		encToChunk:	fn(this: self ref Shape)			 : array of byte;

	};

#	init: fn(): array of ref Level;

	rootCanvas:  fn()                 : ref Shape;

	newSVG:      fn(svg: string)      : ref Shape;
	newPNG:      fn(png_file_name: string) : ref Shape;
	newShape:    fn()                 : ref Shape;

	newFromChunk: fn(chunk: array of byte) : ref Shape;

	newCanvas:   fn()                 : ref Shape;


#	shapeLineWidth: fn(id: int, w: real);
#	shapeLineColor: fn(id: int, r: real, g: real, b: real, a: real);
#	shapeFillColor: fn(id: int, r: real, g: real, b: real, a: real);

#	shapeMoveTo:    fn(id: int, x: real, y: real);
#	shapeLineTo:    fn(id: int, x: real, y: real);

	moveTo:     fn(id: int, x: int, y: int);

#	setToPos:   fn(id: int, n: int)  : int;
#	insToPos:   fn(id: int, n: int)  : int;
#	getFromPos: fn(n: int)           : int;
#	rmFromPos:  fn(n: int)           : int;

	destroyObj: fn(id: int);

#	getLevelsTopPos: fn()       : int;

	getLevelsList:   fn()            : array of int;



	doRender:     fn();



	increment: fn(i: int): int;
	say: fn(s: string);

	tst: fn(s: string);

};
