import { Component, OnInit } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { FormControl, FormGroup } from '@angular/forms';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-calibrate-dialog',
  templateUrl: './calibrate-dialog.component.html',
  styleUrls: ['./calibrate-dialog.component.css']
})
export class CalibrateDialogComponent implements OnInit {

  form = new FormGroup({
    invertPitchAxis: new FormControl(false)
  });

  calibrateLevel = true;
  calibrateTipped = false;

  constructor(
    private dialogRef: MatDialogRef<CalibrateDialogComponent>,
    private model: ModelService) {
      this.form.setValue({
        invertPitchAxis: false
      });
  }

  ngOnInit(): void {
  }

  calibrate(): void {
    if (this.calibrateLevel) {
      this.model.io.emit('calibrateLevel', (res: any) => {
        console.log('calibrateLevel:', res);
        if (res) {
          this.calibrateLevel = false;
          this.calibrateTipped = true;
        }
      });
    } else if (this.calibrateTipped) {
      this.model.io.emit('calibrateTipped', this.form.value, (res: any) => {
        console.log('calibrateTipped:', res);
        if (res)
          this.dialogRef.close(true);
      });
    }
  }

  cancel(): void {
    this.dialogRef.close(false);
  }

}
