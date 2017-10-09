package maxweight;


public class MaxWeight {

    public static void main(String[] args) {
       int sTime[]={1,3,6,2};
       int fTime[]={2,5,19,100};
       int weight[]={50,30,100,200};
       int n=sTime.length;
       System.out.println(mw(sTime,fTime,weight,n));
      
    }

    static int mw(int[] sTime, int[] fTime, int[] weight, int n) {
        int i,j,max=0;
        int submw[]=new int[n];
        submw[0]=weight[0];
        for(i=1;i<n;i++){
            submw[i]=0;
        }
        
        for(i=1;i<n;i++){
            for(j=0;j<i;j++){
                if(sTime[i]>=fTime[j] && submw[i]<submw[j]+weight[i])
                    submw[i]=submw[j]+weight[i];
            }
        }
        
        for ( i = 0; i < n; i++ )
            if ( max < submw[i] )
                max = submw[i];
        return max;
    }
    
    
    
}
