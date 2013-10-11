package visualise;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

public class WritegraphReader {
    
    private WritegraphReader(){
    }
    
    public static Graph readGraph(InputStream is) throws IOException {
        Graph graph = new Graph();
        Scanner s = new Scanner(is);
        String header = s.nextLine();
        if(!header.startsWith(">>writegraph2d")){
            System.err.println("Incorrect header -- exiting!");
            System.exit(1);
        }
        String nextLine = s.nextLine().trim();
        List<String> lines = new ArrayList<>();
        while(!nextLine.equals("0")){
            lines.add(nextLine);
            nextLine = s.nextLine().trim();
        }
        
        //create vertices
        graph.vertices = new ArrayList<>();
        for(int i=0; i<lines.size(); i++){
            Vertex v = new Vertex();
            v.number = i+1;
            graph.vertices.add(v);
        }
        for(int i=0; i<lines.size(); i++){
            Scanner scanner = new Scanner(lines.get(i));
            if(scanner.nextInt()!=i+1){
                System.err.println("Wrong vertex -- exiting!");
                System.exit(1);
            }
            Vertex v = graph.vertices.get(i);
            v.x = scanner.nextDouble();
            v.y = scanner.nextDouble();
            
            v.edges = new ArrayList<>();
            while(scanner.hasNextInt()){
                Edge e = new Edge();
                e.start = v;
                e.end = graph.vertices.get(scanner.nextInt()-1);
                v.edges.add(e);
            }
        }
        return graph;
    }
}