package visualise;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.GradientPaint;
import java.awt.Graphics2D;
import java.awt.RenderingHints;

public class GraphPainter {
    
    
    private static final Color DEFAULT_VERTEX_COLOR = new Color(250, 178, 126);
    private static final Color DEFAULT_EDGE_COLOR = Color.BLACK;
    private static final Color DEFAULT_NUMBER_COLOR = new Color(64, 64, 255);
    
    private final int width;
    private final int height;
    
    private final int edgeWidth;
    
    private final int vertexSize;
    
    private boolean showVertexNumbers;
    
    private final double rotation;
    private double xMin, xMax, yMin, yMax;
    private double scale, delta, horOffset, verOffset;
    private double horMin, horMax, verMin, verMax;
    private int horSign, verSign;
    
    private final Graph graph;

    public GraphPainter(int width, int height, int edgeWidth, int vertexSize, double rotation, boolean showVertexNumbers, int margin, Graph graph) {
        this.width = width;
        this.height = height;
        this.edgeWidth = edgeWidth;
        this.vertexSize = vertexSize;
        this.rotation = rotation;
        this.showVertexNumbers = showVertexNumbers;
        this.graph = graph;
        calculateBoundingBox();
        setPaintArea(margin + (vertexSize - 1) / 2,
                width - margin - vertexSize / 2,
                height - margin - (vertexSize - 1) / 2,
                margin + vertexSize/ 2);
        calculateCoordinates();
    }
    
    private void calculateBoundingBox(){
        double angle = rotation*Math.PI/180;
        xMin = xMax = graph.vertices.get(0).x*Math.cos(angle)-graph.vertices.get(0).y*Math.sin(angle);
        yMin = yMax = graph.vertices.get(0).x*Math.sin(angle)+graph.vertices.get(0).y*Math.cos(angle);
        for (Vertex v : graph.vertices) {
            double rotatedX = v.x*Math.cos(angle)-v.y*Math.sin(angle);
            double rotatedY = v.x*Math.sin(angle)+v.y*Math.cos(angle);
            xMin = Math.min(xMin, rotatedX);
            xMax = Math.max(xMax, rotatedX);
            yMin = Math.min(yMin, rotatedY);
            yMax = Math.max(yMax, rotatedY);
        }
    }
    
    private void setPaintArea(double horMinValue, double horMaxValue, double verMinValue, double verMaxValue) {
        if (horMinValue <= horMaxValue) {
            horSign = +1;
            horMin = horMinValue;
            horMax = horMaxValue;
        } else {
            horSign = -1;
            horMin = horMaxValue;
            horMax = horMinValue;
        }
        if (verMinValue <= verMaxValue) {
            verSign = +1;
            verMin = verMinValue;
            verMax = verMaxValue;
        } else {
            verSign = -1;
            verMin = verMaxValue;
            verMax = verMinValue;
        }
    }
    
    private void calculateCoordinates() {
        if (horSign != 0 && verSign != 0) {
            double horRng = horMax - horMin;
            double verRng = verMax - verMin;
            if (xMin == xMax || yMin == yMax) {
                delta = Math.max((xMax - xMin) / verRng, (yMax - yMin) / horRng) / 1e6;
            } else {
                delta = Math.min((xMax - xMin) / horRng, (yMax - yMin) / verRng) / 1e6;
            }
            scale = Math.max(delta, Math.min(horRng / (xMax - xMin + delta), verRng / (yMax - yMin + delta)));
            horOffset = (xMin + xMax + delta * horSign) / 2 * scale * horSign - horRng / 2 - horMin;
            verOffset = (yMin + yMax + delta * verSign) / 2 * scale * verSign - verRng / 2 - verMin;
            
            for(Vertex v : graph.vertices){
                v.p = getPoint(v.x, v.y);
            }
        }
    }
    
    private Point getPoint(double x, double y) {
        double angle = rotation*Math.PI/180;
        double rotatedX = x*Math.cos(angle)-y*Math.sin(angle);
        double rotatedY = x*Math.sin(angle)+y*Math.cos(angle);
        Point point = new Point();
        point.x = Math.round((rotatedX * scale * horSign - horOffset - horMin) / delta) * delta + horMin;
        point.y = Math.round((rotatedY * scale * verSign - verOffset - verMin) / delta) * delta + verMin;
        return point;
    }
    
        private static void beginGraph(Graphics2D graphics) {
        //turn on antialiasing
        ((Graphics2D)graphics).setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
    }

    private void beginEdges(Graphics2D graphics) {
        graphics.setColor(DEFAULT_EDGE_COLOR);
    }

    private void paintEdge(Graphics2D graphics, double x1, double y1, double x2, double y2) {
        int xp1, yp1, xp2, yp2;
        xp1 = (int) Math.floor(x1);
        yp1 = (int) Math.floor(y1);
        xp2 = (int) Math.floor(x2);
        yp2 = (int) Math.floor(y2);
        if (edgeWidth == 1) {
            graphics.drawLine(xp1, yp1, xp2, yp2);
        } else {
            double w = edgeWidth + 0.2;
            double dx = yp1 - yp2, dy = xp2 - xp1;
            double d = Math.sqrt(dx * dx + dy * dy);
            dx /= d;
            dy /= d;
            int[] px = new int[4];
            int[] py = new int[4];
            px[1] = (int) Math.round(xp1 - dx * w / 2);
            py[1] = (int) Math.round(yp1 - dy * w / 2);
            px[2] = (int) Math.round(xp2 - dx * w / 2);
            py[2] = (int) Math.round(yp2 - dy * w / 2);
            px[0] = (int) Math.round(px[1] + dx * w);
            py[0] = (int) Math.round(py[1] + dy * w);
            px[3] = (int) Math.round(px[2] + dx * w);
            py[3] = (int) Math.round(py[2] + dy * w);
            graphics.fillPolygon(px, py, 4);
        }
    }

    private void beginVertices(Graphics2D graphics) {
        if(showVertexNumbers){
            Font f = determineVertexFont(graphics);
            if(f==null){
                System.err.println("WARNING: Vertex size too small to show vertex numbers!");
                showVertexNumbers = false;
            } else {
                graphics.setFont(f);
            }
        }
    }

    private void paintVertex(Graphics2D graphics, int number, double x, double y) {
        int xp = (int) Math.floor(x), yp = (int) Math.floor(y);
        if (edgeWidth > 0) {
            graphics.setColor(DEFAULT_EDGE_COLOR);
            graphics.fillOval(xp - (edgeWidth - 1) / 2, yp - (edgeWidth - 1) / 2, edgeWidth, edgeWidth);
        }
        
        final int vertexDiameter = vertexSize;
        final int vertexCornerX = xp - vertexDiameter/2;
        final int vertexCornerY = yp - vertexDiameter/2;
        
        //calculate the two endpoints of the gradient
        final Color colorA = new Color(
                Math.min(DEFAULT_VERTEX_COLOR.getRed() + 4, 255),
                Math.max(DEFAULT_VERTEX_COLOR.getGreen() - 14, 0),
                Math.max(DEFAULT_VERTEX_COLOR.getBlue() - 26, 0));
        final Color colorB = new Color(
                Math.max(DEFAULT_VERTEX_COLOR.getRed() - 4, 0),
                Math.min(DEFAULT_VERTEX_COLOR.getGreen() + 14, 255),
                Math.min(DEFAULT_VERTEX_COLOR.getBlue() + 26, 255));
        
        ((Graphics2D)graphics).setPaint(
                new GradientPaint(
                        vertexCornerX, vertexCornerY, colorA, //from color A in upper left corner
                        vertexCornerX + vertexDiameter, vertexCornerY+vertexDiameter, colorB)); //to color B in lower right corner
        
        graphics.fillOval(vertexCornerX, vertexCornerY, vertexDiameter, vertexDiameter);
        graphics.setColor(Color.BLACK);
        graphics.drawOval(vertexCornerX, vertexCornerY, vertexDiameter, vertexDiameter);
        if (showVertexNumbers) {
            String numberString = Integer.toString(number);
            graphics.setColor(DEFAULT_NUMBER_COLOR);
            int stringWidth = graphics.getFontMetrics().stringWidth(numberString);
            graphics.drawString(numberString,
                    xp - (int) Math.floor(stringWidth * 0.52),
                    yp + (int) Math.floor(graphics.getFontMetrics().getAscent() * 0.47));
        }
    }

    private void endGraph(Graphics2D graphics) {
        //do nothing
    }

    public void paintGraph(Graphics2D graphics){
        beginGraph(graphics);

        beginEdges(graphics);
        for (Vertex v : graph.vertices) {
            for (Edge e : v.edges){
                if(e.end.number <= e.start.number){
                    paintEdge(graphics, e.start.p.x, e.start.p.y, e.end.p.x, e.end.p.y);
                }
            }
        }
        beginVertices(graphics);
        for (Vertex v : graph.vertices) {
            paintVertex(graphics, v.number, v.p.x, v.p.y);
        }
            
        endGraph(graphics);
    }
    
    private Font determineVertexFont(Graphics2D graphics) {
        Font vertexFont = graphics.getFont();
        int fontSize = getVertexFontSize(graphics);
        return new Font(
                vertexFont.getName(),
                vertexFont.getStyle() & Font.BOLD,
                fontSize);
    }

    private int getVertexFontSize(Graphics2D graphics) {
        Font vertexFont = graphics.getFont();
        FontMetrics fm = graphics.getFontMetrics(vertexFont);
        int w = fm.stringWidth(Integer.toString(graph.vertices.size()));
        int h = fm.getAscent();
        int fontSize;
        double factor = vertexSize * 0.85 / Math.sqrt(w * w + h * h);
        if (h * factor < 7.5) {
            fontSize = 0;
        } else {
            fontSize = (int) Math.round(vertexFont.getSize() * factor);
        }
        return fontSize;
    }
}