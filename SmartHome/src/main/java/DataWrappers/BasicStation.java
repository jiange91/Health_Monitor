package DataWrappers;

public class BasicStation {
    public int health_score;
    public boolean status_sitting;
    public boolean status_sedentary;
    public int cp;
    public int ucp;
    public boolean lack_of_shift;
    public boolean stand_slow;

    public BasicStation(int health_score, boolean status_sitting, boolean status_sedentary, int cp, int ucp,
                        boolean lack_of_shift, boolean stand_slow){
        this.health_score = health_score;
        this.status_sitting = status_sitting;
        this.status_sedentary = status_sedentary;
        this.cp = cp;
        this.ucp = ucp;
        this.lack_of_shift = lack_of_shift;
        this.stand_slow = stand_slow;
    }
}
