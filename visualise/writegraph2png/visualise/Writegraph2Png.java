package visualise;

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
        if(System.getProperties().containsKey("help")){
            System.err.println("HELP!");
        }
        width = readIntProperty("width", width);
        height = readIntProperty("height", height);
        edgeWidth = readIntProperty("edgeWidth", edgeWidth);
        vertexSize = readIntProperty("vertexSize", vertexSize);
        rotation = readDoubleProperty("rotation", rotation);
        showVertexNumbers = detectProperty("numbers");
    }

    public static void main(String[] args) {
        readProperties();
        try {
            Graph graph = WritegraphReader.readGraph(System.in);
            GraphPainter painter = new GraphPainter(width, height, edgeWidth, vertexSize, rotation, showVertexNumbers, 5, graph);
            saveFile(new File(args.length == 0 ? "image.png" : args[0]), painter);
        } catch (IOException ex) {
            Logger.getLogger(Writegraph2Png.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
}