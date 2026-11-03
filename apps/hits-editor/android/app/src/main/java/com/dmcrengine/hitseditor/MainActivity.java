package com.dmcrengine.hitseditor;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.net.Uri;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.widget.*;
import java.io.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class MainActivity extends Activity {
    private long handle;
    private boolean scmMode;
    private int preset;
    private byte[] pendingSave;
    private TextView status;
    private EditorView viewport;
    private final ExecutorService io = Executors.newSingleThreadExecutor();

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        handle=Native.create();
        LinearLayout root=new LinearLayout(this);root.setOrientation(LinearLayout.VERTICAL);
        // API 35+ enforces edge-to-edge: account for system bars and cutouts.
        root.setOnApplyWindowInsetsListener((view,insets)->{
            if(android.os.Build.VERSION.SDK_INT>=30){
                android.graphics.Insets bars=insets.getInsets(android.view.WindowInsets.Type.systemBars()|android.view.WindowInsets.Type.displayCutout());
                view.setPadding(bars.left,bars.top,bars.right,bars.bottom);
            }else view.setPadding(insets.getSystemWindowInsetLeft(),insets.getSystemWindowInsetTop(),insets.getSystemWindowInsetRight(),insets.getSystemWindowInsetBottom());
            return insets;
        });
        HorizontalScrollView scroll=new HorizontalScrollView(this);
        LinearLayout controls=new LinearLayout(this);
        add(controls,"Open HITS",()->confirmDiscard(()->openPicker(1)));
        add(controls,"Open SCM",()->openPicker(2));
        add(controls,"Export HITS",this::export);
        add(controls,"Fit",()->action(0));
        add(controls,"Connected",()->action(1));
        add(controls,"Undo",()->action(3));add(controls,"Redo",()->action(4));
        add(controls,"Pick HITS / SCM",()->{scmMode=!scmMode;refresh();});
        add(controls,"SCM → HITS",()->action(5));
        add(controls,"Move",()->geometryDialog(false));
        add(controls,"Boundary",()->geometryDialog(true));
        scroll.addView(controls);root.addView(scroll);
        LinearLayout colors=new LinearLayout(this);
        String[] labels={"Blue","Orange","Green","Red"};int[] rgb={0x4285F4,0xFF9933,0x40CB78,0xEF5350};
        for(int i=0;i<4;i++){final int p=i;Button b=new Button(this);b.setText(labels[i]);b.setTextColor(0xFF000000|rgb[i]);
            b.setOnClickListener(v->{preset=p;action(2);});colors.addView(b,new LinearLayout.LayoutParams(0,-2,1));}
        root.addView(colors);
        viewport=new EditorView();root.addView(viewport,new LinearLayout.LayoutParams(-1,0,1));
        status=new TextView(this);status.setTextSize(12);root.addView(status);
        setContentView(root);refresh();
    }
    private void add(LinearLayout row,String label,Runnable task){Button b=new Button(this);b.setText(label);b.setOnClickListener(v->{try{task.run();}catch(RuntimeException e){error(e.getMessage());}});row.addView(b);}
    private void refresh(){if(handle!=0){status.setText(Native.status(handle)+" | Pick "+(scmMode?"SCM":"HITS")+" | Drag orbit / pinch zoom");viewport.invalidate();}}
    private void error(String message){new AlertDialog.Builder(this).setTitle("HITS Editor").setMessage(message==null?"Operation failed":message).setPositiveButton("OK",null).show();}
    private void action(int action){if(!Native.action(handle,action,preset))Toast.makeText(this,"Select a surface/object first, or open HITS",Toast.LENGTH_SHORT).show();refresh();}
    private void confirmDiscard(Runnable next){if(!Native.dirty(handle)){next.run();return;}new AlertDialog.Builder(this).setMessage("Discard unsaved HITS edits?").setPositiveButton("Discard",(d,w)->next.run()).setNegativeButton("Cancel",null).show();}
    private void openPicker(int request){Intent i=new Intent(Intent.ACTION_OPEN_DOCUMENT);i.setType("*/*");i.addCategory(Intent.CATEGORY_OPENABLE);startActivityForResult(i,request);}
    private void export(){pendingSave=Native.save(handle);if(pendingSave==null){error("Open HITS first. Canonical rebuild must succeed before export.");return;}Intent i=new Intent(Intent.ACTION_CREATE_DOCUMENT);i.setType("application/octet-stream");i.addCategory(Intent.CATEGORY_OPENABLE);i.putExtra(Intent.EXTRA_TITLE,"edited.hits");startActivityForResult(i,3);}
    private void geometryDialog(boolean boundary){EditText input=new EditText(this);input.setSingleLine();input.setHint(boundary?"minX minY minZ maxX maxY maxZ":"deltaX deltaY deltaZ");
        new AlertDialog.Builder(this).setTitle(boundary?"Rectangular boundary (8 triangles)":"Translate selected surfaces").setView(input)
            .setPositiveButton("Apply",(d,w)->{try{String[] tokens=input.getText().toString().trim().split("[\\s,;]+");if(tokens.length!=(boundary?6:3))throw new IllegalArgumentException("Enter "+(boundary?6:3)+" numbers");float[] v=new float[tokens.length];for(int j=0;j<v.length;j++){v[j]=Float.parseFloat(tokens[j]);if(!Float.isFinite(v[j]))throw new IllegalArgumentException("Coordinates must be finite");}if(!Native.geometry(handle,v,boundary,preset))throw new IllegalArgumentException("Invalid geometry or empty selection");refresh();}catch(RuntimeException e){error(e.getMessage());}})
            .setNegativeButton("Cancel",null).show();}
    @Override protected void onActivityResult(int request,int result,Intent data){super.onActivityResult(request,result,data);if(result!=RESULT_OK||data==null){if(request==3)pendingSave=null;return;}Uri uri=data.getData();if(uri==null)return;
        if(request==3){final byte[] bytes=pendingSave;pendingSave=null;if(bytes==null)return;io.execute(()->{try(OutputStream out=getContentResolver().openOutputStream(uri,"wt")){if(out==null)throw new IOException("Cannot create document");out.write(bytes);out.flush();runOnUiThread(()->{if(!isDestroyed())Toast.makeText(this,"HITS exported",Toast.LENGTH_LONG).show();});}catch(IOException e){runOnUiThread(()->{if(!isDestroyed())error("Export failed: "+e.getMessage());});}});return;}
        io.execute(()->{try(InputStream in=getContentResolver().openInputStream(uri);ByteArrayOutputStream out=new ByteArrayOutputStream()){
            if(in==null)throw new IOException("Cannot open document");byte[] block=new byte[65536];int n;while((n=in.read(block))!=-1){if(out.size()+n>256*1024*1024)throw new IOException("File exceeds 256 MiB");out.write(block,0,n);}byte[] bytes=out.toByteArray();
            runOnUiThread(()->{if(handle==0||isDestroyed())return;try{if(!Native.open(handle,bytes,request==2))error("Invalid or unsupported resource");refresh();}catch(RuntimeException e){error(e.getMessage());}});
        }catch(IOException e){runOnUiThread(()->{if(!isDestroyed())error(e.getMessage());});}});
    }
    @Override public void onBackPressed(){confirmDiscard(this::finish);}
    @Override protected void onDestroy(){super.onDestroy();if(handle!=0){Native.destroy(handle);handle=0;}io.shutdown();}

    private final class EditorView extends View {
        private final Paint paint=new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Path path=new Path();
        private final ScaleGestureDetector scale;
        private float lastX,lastY,startX,startY;
        private boolean moved;
        EditorView(){super(MainActivity.this);scale=new ScaleGestureDetector(MainActivity.this,new ScaleGestureDetector.SimpleOnScaleGestureListener(){@Override public boolean onScale(ScaleGestureDetector d){Native.camera(handle,0,0,1/d.getScaleFactor());invalidate();return true;}});}
        @Override protected void onDraw(Canvas canvas){canvas.drawColor(Color.rgb(20,24,32));if(handle==0)return;
            try{float[] data=Native.frame(handle,getWidth(),getHeight());if(data==null)return;
                for(int i=0;i+7<data.length;i+=8){path.reset();path.moveTo(data[i],data[i+1]);path.lineTo(data[i+2],data[i+3]);path.lineTo(data[i+4],data[i+5]);path.close();paint.setStyle(Paint.Style.FILL);paint.setColor(0xFF000000|(int)data[i+6]);canvas.drawPath(path,paint);paint.setStyle(Paint.Style.STROKE);paint.setStrokeWidth(data[i+7]>0?3:1);paint.setColor(data[i+7]>0?Color.WHITE:0xFF262C36);canvas.drawPath(path,paint);}
            }catch(RuntimeException e){status.setText("Viewport failed: "+e.getMessage());}}
        @Override public boolean onTouchEvent(MotionEvent e){if(handle==0)return false;scale.onTouchEvent(e);
            switch(e.getActionMasked()){
                case MotionEvent.ACTION_DOWN:lastX=startX=e.getX();lastY=startY=e.getY();moved=false;return true;
                case MotionEvent.ACTION_POINTER_DOWN:moved=true;return true;
                case MotionEvent.ACTION_MOVE:if(Math.hypot(e.getX()-startX,e.getY()-startY)>8)moved=true;
                    if(e.getPointerCount()==1&&!scale.isInProgress()&&moved){Native.camera(handle,(e.getX()-lastX)*0.008f,(e.getY()-lastY)*0.008f,1);invalidate();}lastX=e.getX();lastY=e.getY();return true;
                case MotionEvent.ACTION_UP:if(!moved){Native.pick(handle,e.getX(),e.getY(),getWidth(),getHeight(),scmMode);performClick();refresh();}return true;
                default:return true;
            }}
        @Override public boolean performClick(){super.performClick();return true;}
    }
}
