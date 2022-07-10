import { Component } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { MatDialog } from '@angular/material/dialog';
import { Subscription } from 'rxjs';
import { ModelService } from './model.service';
import { SettingsDialogComponent } from './settings-dialog/settings-dialog.component';
import { RebootDialogComponent } from './reboot-dialog/reboot-dialog.component';
import { CalibrateDialogComponent } from './calibrate-dialog/calibrate-dialog.component';
import { PairDialogComponent } from './pair-dialog/pair-dialog.component';
import { UnpairDialogComponent } from './unpair-dialog/unpair-dialog.component';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {

  title: string = '';
  //mode: string = '';
  //name: string = '';
  //configDirty: boolean = false;

  private modeSubscription!: Subscription;
  private nameSubscription!: Subscription;
  //private configDirtySubscription!: Subscription;

  constructor(
    private titleService: Title,
    public model: ModelService,
    private dialog: MatDialog
  ) {}

  ngOnInit(): void {
    this.modeSubscription = this.model.mode.asObservable().subscribe(s => {this.setTitle()});
    /*
    this.modeSubscription = this.modelService.mode$.subscribe(s => {
      this.mode = s;
      this.setTitle();
    });
    */
    this.nameSubscription = this.model.name.asObservable().subscribe(s => {this.setTitle()});
    //this.configDirtySubscription = this.modelService.configDirty$.subscribe(b => this.configDirty = b)
  }

  ngOnDestroy() {
    this.modeSubscription.unsubscribe();
    this.nameSubscription.unsubscribe();
    //this.configDirtySubscription.unsubscribe();
  }

  setTitle(): void {
    this.title = this.model.mode.value;
    if (this.model.name.value) this.title += ': ' + this.model.name.value;
    this.titleService.setTitle(this.title);
  }

  settings(): void {
    this.dialog.open(SettingsDialogComponent);
  }

  calibrate(): void {
    this.dialog.open(CalibrateDialogComponent);
  }
  
  pair(): void {
    this.dialog.open(PairDialogComponent);
  }
  
  unpair(): void {
    this.dialog.open(UnpairDialogComponent);
  }
  
  saveConfig(): void {
    this.model.saveConfig().subscribe({
      next: (res: string) => console.info('saveConfig: ' + res)
    });
  }

  reboot(): void {
    this.dialog.open(RebootDialogComponent);
  }

}
