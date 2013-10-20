package visualise;

import java.awt.Color;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.imageio.ImageIO;


public class Writegraph2Png {

    private Writegraph2Png() {
    }
    
    private static int width = 200;
    private static int height = 200;
    private static int edgeWidth = 3;
    private static int vertexSize = 8;
    private static double rotation = 0.0;
    private static boolean showVertexNumbers = false;
    private static Color vertexColor = null;
    private static Color edgeColor = null;
    private static Color numberColor = null;
    
    private static void saveFile(File file, GraphPainter painter) {
        BufferedImage im = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        
        painter.paintGraph(im.createGraphics());
        
        try {
            ImageIO.write(im, "PNG", file);
        } catch (IOException ex) {
            Logger.getLogger(Writegraph2Png.class.getName()).log(Level.SEVERE, 
                    "Error while writing file", ex);
        }
    }
    
    private static int readIntProperty(String key, int defaultValue){
        if(System.getProperties().containsKey(key)){
            return Integer.parseInt(System.getProperties().getProperty(key));
        } else {
            return defaultValue;
        }
    }
    
    private static int getValidColorComponent(String s){
        return Math.max(Math.min(Integer.parseInt(s),255), 0);
    } 
    
    private static Color readColorProperty(String key){
        if(System.getProperties().containsKey(key)){
            String color = System.getProperties().getProperty(key);
            String[] parts = color.split(",");
            if(parts.length==3){
                return new Color(getValidColorComponent(parts[0]), 
                                 getValidColorComponent(parts[1]), 
                                 getValidColorComponent(parts[2]));
            } else {
                return null;
            }
        } else {
            return null;
        }
    }
    
    private static double readDoubleProperty(String key, double defaultValue){
        if(System.getProperties().containsKey(key)){
            return Double.parseDouble(System.getProperties().getProperty(key));
        } else {
            return defaultValue;
        }
    }
    
    private static boolean detectProperty(String key){
        return System.getProperties().containsKey(key);
    }
    
    private static void readProperties(){
        if(detectProperty("help")){
            printHelp();
            System.exit(0);
        }
        width = readIntProperty("width", width);
        height = readIntProperty("height", height);
        edgeWidth = readIntProperty("edge-width", edgeWidth);
        vertexSize = readIntProperty("vertex-size", vertexSize);
        rotation = readDoubleProperty("rotation", rotation);
        showVertexNumbers = detectProperty("numbers");
        vertexColor = readColorProperty("vertex-color");
        edgeColor = readColorProperty("edge-color");
        numberColor = readColorProperty("number-color");
    }
    
    private static void printHelp(){
        String message =
"This program reads a graph in writegraph2d format and writes a PNG image.\n\n" +
"Usage\n=====\n" +
"java [options] -jar writegraph2png.jar [name.png]\n\n" +
"If no filename is given, then image.png is used. The graph is always read from\n" +
"standard in. Make sure that you place the options at the right position, i.e.,\n" +
"before the -jar option. At the moment this is import.\n\n" +
"Valid options\n=============\n" +
"All options always start with -D!\n" +
"    -Dhelp\n" +
"       Print this help and return.\n" +
"    -Dwidth=#\n" +
"       Set the width of the image to #.\n" +
"    -Dheight=#\n" +
"       Set the height of the image to #.\n" +
"    -Dedge-width=#\n" +
"       Set the thickness of the lines representing the edges to #.\n" +
"    -Dvertex-size=#\n" +
"       Set the diameter of the circles representing the vertices to #.\n" +
"    -Drotation=#\n" +
"       Rotate the image by # degrees.\n" +
"    -Dnumbers\n" +
"       Show vertex numbers. If the vertices are too small, the numbers are not\n" +
"       shown and a warning is printed.\n" +
"    -Dvertex-color=#,#,#\n" +
"       Set the color of the vertices as RGB values in the range [0,255].\n" +
"    -Dedge-color=#,#,#\n" +
"       Set the color of the edges as RGB values in the range [0,255].\n" +
"    -Dnumber-color=#,#,#\n" +
"       Set the color of the vertex numbers as RGB values in the range [0,255].\n";
        System.err.println(message);
    }

    public static void main(String[] args) {
        readProperties();
        try {
            Graph graph = WritegraphReader.readGraph(System.in);
            GraphPainter painter = new GraphPainter(width, height, edgeWidth, vertexSize, rotation, showVertexNumbers, 5, graph);
            painter.setVertexColor(vertexColor);
            painter.setEdgeColor(edgeColor);
            painter.setNumberColor(numberColor);
            saveFile(new File(args.length == 0 ? "image.png" : args[0]), painter);
        } catch (IOException ex) {
            Logger.getLogger(Writegraph2Png.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
}