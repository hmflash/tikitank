diameter=3.04*25.4;  // Ring inner diameter
height=0.5*25.4;     // Ring height
thickness=5;         // Ring thickness
mag_dia=6.10;        // Magnet diameter
num_mags=16;         // Number of magnets
detail=100;          // Circle detail
teeth=2;             // Joint teeth count

difference(){
    difference() {
        rotate_extrude($fn=detail)
        translate([diameter / 2, 0, 0])
        square([thickness,height]);
        for (i = [0 : num_mags / 2]) {
            translate([0,0,height])
            rotate([90,0,(i+0.5) * (360 / num_mags)])
            cylinder(diameter*2, r=mag_dia/2, $fn=detail, center=true);
        }
    }
    union() {
        for (i = [0 : teeth - 1]) {
            s = -height/teeth/2;
            translate([0,s/2,i*height/teeth-s])
            rotate([-45,0,0])
            cube([diameter,diameter,diameter]);
        }
        for (i = [0 : teeth]) {
            s = -height/teeth/2;
            translate([-diameter,s/2,i*height/teeth])
            rotate([-45,0,0])
            cube([diameter,diameter,diameter]);
        }
    }
}