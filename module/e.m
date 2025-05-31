E: module
{
	PATH: con "$E";

	XY_coord : adt {
		x, y : real;
	};

	Arc_coord : adt {
		center : XY_coord;
		radius : real;
		ang_from : real;
		ang_to   : real;
	};

	GraphOp : adt {
		op : string;
	
		pick
		{
			MOVE_TO  => coord: XY_coord;
			LINE_TO  => coord: XY_coord;
			CURVE_TO => coord: (XY_coord, XY_coord, XY_coord);
			ARC_CW   => coord: Arc_coord;
			ARC_CCW  => coord: Arc_coord;
			CLOSE_PATH => coord: int;
		}
	};


	ShapeEvent : adt {
		shape : ref Shape;

		pick
		{
			MOUSE  => data  : (string, XY_coord, XY_coord);
			TOUCH  => data  : array of XY_coord;
		}
	};


	
	Level : adt {
		note: string;

#		graphics : GrOp;

		new:	fn(): ref Level;

#		getPos: fn(): int;
#		insPos: fn(n: int);
#		setPos: fn(n: int): Level;
#		swpPos: fn(n: int);

#		rmPos : fn();

	};

	Shape : adt {
		id : int;

		graph_ops : array of ref GraphOp;

		exec_cb_path : string;

		sub_canvas : array of ref Shape;

		mask_canvas : array of ref Shape;

		parent : array of ref Shape;


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

		getSubCanvasLen:	fn(this: self ref Shape) : int;
		getSubCanvasItm:	fn(this: self ref Shape, i: int) : ref Shape;

		getMaskedCanvasLen:	fn(this: self ref Shape) : int;
		getMaskedCanvasItm:	fn(this: self ref Shape, i: int) : ref Shape;

		getParent:      fn(this: self ref Shape) : ref Shape;

		updGraph:		fn(this: self ref Shape);
		undoGraph:		fn(this: self ref Shape);

		encToChunk:	fn(this: self ref Shape)			 : array of byte;

		setExecCbPath: fn(this: self ref Shape, new_path: string): (string, int);
		setExecCbChan: fn(this: self ref Shape, c: chan of ref ShapeEvent);
		getExecCbChan: fn(this: self ref Shape): chan of ref ShapeEvent;

	};

#	init: fn(): array of ref Level;

	rootCanvas:  fn()                 : ref Shape;

	newSVG:      fn(svg: string)      : ref Shape;
	newPNG:      fn(png_file_name: string) : ref Shape;
	newShape:    fn()                 : ref Shape;

	newFromChunk: fn(chunk: array of byte) : ref Shape;

	newCanvas:   fn()                 : ref Shape;

	moveTo:     fn(id: int, x: int, y: int);

	destroyObj: fn(id: int);


	getLevelsList:   fn()            : array of int;



	doRender:     fn();



	increment: fn(i: int): int;
	say: fn(s: string);

	tst: fn(s: string);

};
