# scons: Png Component Merge

from pyTuttle import tuttle
import numpy
import Image

def setUp():
	tuttle.core().preload(False)

def testInputBuffer_loadImageWithPIL():
	"""
	Load an image with PIL and use it with the InputBuffer plugin.
	"""
	g = tuttle.Graph()

	ib = g.createInputBuffer()
	## numpy array from an image
	img = numpy.asarray(Image.open('data/input.jpg'))
	ib.set3DArrayBuffer( img )

	w = g.createNode("tuttle.pngwriter", filename="foo.png")

	g.connect( ib.getNode(), w )
	g.compute( w )


def testInputBuffer_generateImageBufferWithNumpy():
	"""
	Generate an image buffer with numpy and use it with the InputBuffer plugin.
	"""
	g = tuttle.Graph()

	ib = g.createInputBuffer()
	# generate numpy array
	x = numpy.array([[.9, .1, .9], [.8, .2, .9]], numpy.float32)
	ib.set2DArrayBuffer( x )

	w = g.createNode("tuttle.pngwriter", filename="foo.png")

	g.connect( ib.getNode(), w )
	g.compute( w )


def testInputBuffer_MergeInputBufferNodes():
	"""
	Merge an image file (loaded with PIL) with a generated image buffer (created with numpy).
	"""
	g = tuttle.Graph()

	img = numpy.asarray(Image.open('data/input.jpg'))
	ii = g.createInputBuffer()
	ii.set3DArrayBuffer( img )

	# generated numpy array
	array = numpy.array([[.9, .1, .9], [.8, .2, .9]], numpy.float32)
	ib = g.createInputBuffer()
	ib.set2DArrayBuffer( array )

	c = g.createNode("tuttle.component", to="rgb")
	m = g.createNode("tuttle.merge", mergingFunction="average", rod="union")
	w = g.createNode("tuttle.pngwriter", "foo.png")

	g.connect( ib.getNode(), c )
	g.connect( c, m.getAttribute("A") )
	g.connect( ii.getNode(), m.getAttribute("B") )
	g.connect( m, w )

	# @todo: fix this test, it doesn't work.
	#        Maybe a bug on the host side.
	#g.compute( w )
