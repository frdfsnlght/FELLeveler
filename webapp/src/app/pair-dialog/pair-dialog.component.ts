import { Component, OnInit } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { Subscription } from 'rxjs';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-pair-dialog',
  templateUrl: './pair-dialog.component.html',
  styleUrls: ['./pair-dialog.component.css']
})
export class PairDialogComponent implements OnInit {

  mode: string = '';
  btPairedSubscription!: Subscription;
  scanning = false;

  constructor(
    private dialogRef: MatDialogRef<PairDialogComponent>,
    public model: ModelService) {
      this.mode = model.mode.value;
  }

  ngOnInit(): void {
    this.scan();
  }

  scan(): void {
    this.scanning = true;
    this.model.scanBTDevices().subscribe({
      next: (res: string) => {
        console.info('scanBTDevices: ' + res);
      },
      complete: () => {
        this.scanning = false;
      }
    });
  }

  pair(address: string): void {
    this.model.pairBTDevice(address).subscribe({
      next: (res: string) => {
        console.info('pairBTDevice: ' + res);
        if (res == 'OK')
          this.dialogRef.close(true);
      }
    });
  }

  cancel(): void {
    this.dialogRef.close(false);
  }

}
