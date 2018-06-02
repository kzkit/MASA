package zhenkit.masaproject;


import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;
import com.jjoe64.graphview.DefaultLabelFormatter;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.Viewport;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

/**
 * A simple {@link Fragment} subclass.
 */
public class Fifth extends Fragment {
TextView xValue,yValue;
FirebaseDatabase database;
DatabaseReference reference,mDatabaseGig;
View button;
GraphView graphView;
LineGraphSeries series;
int xaxis=0;
int yaxis=0;
    public Fifth() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_fifth, container, false);

        button = view.findViewById(R.id.btn_insert);
        graphView = (GraphView) view.findViewById(R.id.graph55);
        series = new LineGraphSeries();
        graphView.addSeries(series);
        database=FirebaseDatabase.getInstance();
        reference=database.getReference("chartTable");


        // get the gig database
        mDatabaseGig = FirebaseDatabase.getInstance().getReference("heart_rate");

        mDatabaseGig.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String someValue =(String) dataSnapshot.getValue();
                yaxis = Integer.parseInt(someValue);
            }
            @Override
            public void onCancelled(DatabaseError databaseError) {

            }
        });

        //Button
        button.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view){
                xaxis++;
                if(xaxis > 10) {
                    xaxis = 0;
                    reference.removeValue();
                    Toast.makeText(getActivity(), "hello55", Toast.LENGTH_SHORT).show();
                }
                String id = "data"+xaxis;
                int x=xaxis;
                int y= yaxis;

                PointValue pointValue = new PointValue(x,y);
                reference.child(id).setValue(pointValue);
            }
        });

        reference.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                DataPoint[] dp = new DataPoint[(int)dataSnapshot.getChildrenCount()];
                int index=0;

                for (DataSnapshot myDataSnapshot : dataSnapshot.getChildren()){
                    PointValue pointValue = myDataSnapshot.getValue(PointValue.class);
                    dp[index]= new DataPoint(pointValue.getxValue(),pointValue.getyValue());
                    index++;
                }
                series.resetData(dp);
                }


            @Override
            public void onCancelled(DatabaseError databaseError) {

            }
        });


        return view;
    }
}
